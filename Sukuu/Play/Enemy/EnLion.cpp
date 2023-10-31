#include "stdafx.h"
#include "EnLion.h"

#include "Assets.generated.h"
#include "EnemyUtil.h"
#include "Play/PlayScene.h"
#include "Play/Chara/CharaUtil.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.en_lion." + key);
	}

	constexpr Rect spriteRect{0, 0, 32, 32};
}

struct Play::EnLion::Impl : EnemyTransform
{
	bool m_doingLostPenalty = false;
	bool m_jumpAttacking{};

	void Update()
	{
		m_animTimer.Tick();

		// プレイヤーとの当たり判定
		CheckSendEnemyCollide(PlayScene::Instance().GetPlayer(), *this, EnemyKind::Lion);

		const AssetNameView emotion = [&]()
		{
			if (m_jumpAttacking) return U"🤗";
			if (m_trapped == EnemyTrappedState::Captured) return U"😬";
			if (m_playerTracker.IsTracking()) return U"😎";
			if (m_doingLostPenalty) return U"🤔";
			return U"";
		}();
		DrawEnemyBasically(*this, emotion);
	}

	Vec2 GetDrawPos() const override
	{
		return m_pos.viewPos.movedBy(m_animOffset + GetCharacterCellPadding(spriteRect.size));
	}

	void StartFlowchart(ActorBase& self)
	{
		StartCoro(self, [this, self](YieldExtended yield)
		{
			while (true)
			{
				flowchartLoop(yield, self);
			}
		});
	}

	TextureRegion GetTexture() const override
	{
		auto&& t =
			TextureAsset(AssetImages::lion_32x32)(
				spriteRect.movedBy(m_animTimer.SliceFrames(100, 6) * spriteRect.w, 0));
		return m_dir == Dir4::Left || m_dir == Dir4::Up ? t : t.mirrored();
	}

private:
	void flowchartLoop(YieldExtended& yield, ActorView self)
	{
		yield();

		// 移動中
		const bool leftPriority = RandomBool(0.5);
		while (true)
		{
			auto&& map = PlayScene::Instance().GetMap();
			auto&& gimmick = PlayScene::Instance().GetGimmick();
			const TerrainKind currentTerrain = GetTerrainAt(map, m_pos.actualPos);
			const auto currentPoint = m_pos.actualPos.MapPoint();

			// ギミック確認
			CheckEnemyTrappingGimmick(yield, self, currentPoint, *this);

			// プレイヤー追跡チェック
			m_playerTracker.Track(
				m_dir,
				currentPoint,
				getToml<int>(U"max_player_concern"),
				[&]()
				{
					m_doingLostPenalty = true;
					yield.WaitForTime(getToml<double>(U"lost_player_penalty"));
					m_doingLostPenalty = false;
				});

			// プレイヤーが壁に隠れても攻撃する
			checkJumpAttack(yield, self);

			// 曲がるかチェック
			if (not m_playerTracker.IsTracking())
				checkTurn(map, gimmick, currentTerrain, leftPriority);

			// 進行可能方向に向く
			if (FaceEnemyMovableDir(m_dir, m_pos, map, gimmick, leftPriority) == false) break;

			// 移動開始
			auto nextPos = m_pos.actualPos + m_dir.ToXY() * CellPx_24;
			const double moveDuration = m_playerTracker.IsTracking()
				                            ? getToml<double>(U"tracking_duration")
				                            : getToml<double>(U"move_duration");;
			ProcessMoveCharaPos(yield, self, m_pos, nextPos, moveDuration);
		}
	}

	void checkJumpAttack(YieldExtended& yield, ActorView self)
	{
		auto&& player = PlayScene::Instance().GetPlayer();
		auto&& map = PlayScene::Instance().GetMap();
		if (not m_playerTracker.IsTracking()) return;
		const auto playerPoint = player.CurrentPoint();
		if ((m_pos.actualPos.MapPoint() - playerPoint).manhattanLength() != 1) return;
		yield.WaitForFalse([&]() { return player.IsImmortal(); });
		if (map.Data().inBounds(playerPoint) && map.Data()[playerPoint].kind == TerrainKind::Wall)
		{
			// 壁に向かって攻撃
			m_collideEnabled = false;
			m_jumpAttacking = true;
			const auto beforePos = m_pos.actualPos;
			const double attackDuration = getToml<double>(U"attack_duration");
			AnimateEasing<BoomerangParabola>(self, &m_animOffset, Vec2{0, -32}, attackDuration);
			ProcessMoveCharaPos<EaseInOutBack>(yield, self, m_pos, playerPoint * CellPx_24, attackDuration);
			// 攻撃チェック
			player.SendEnemyCollide({m_pos.actualPos, {CellPx_24, CellPx_24}}, EnemyKind::Lion);
			ProcessMoveCharaPos(yield, self, m_pos, beforePos, attackDuration);
			m_jumpAttacking = false;
			m_collideEnabled = true;
		}
	}

	bool checkTurn(const MapGrid& map, const GimmickGrid& gimmick, TerrainKind currentTerrain, bool leftPriority)
	{
		// 部屋から出れるかチェック
		const double cornerPr = getToml<double>(U"corner_pr");
		if (currentTerrain == TerrainKind::Floor &&
			RandomBool(cornerPr))
		{
			return RotateEnemyDirFacingPathway(m_dir, m_pos, map, leftPriority);
		}

		// 通路を曲がるかチェック
		if (currentTerrain == TerrainKind::Pathway &&
			RandomBool(cornerPr))
		{
			return RotateEnemyDirFacingMovable(m_dir, m_pos, map, gimmick, leftPriority);
		}
		return false;
	}
};

namespace Play
{
	EnLion::EnLion() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void EnLion::Init()
	{
		p_impl->m_pos.SetPos(GetInitialPos(PlayScene::Instance().GetMap()));
		p_impl->StartFlowchart(*this);
	}

	void EnLion::Update()
	{
		ActorBase::Update();
		p_impl->Update();
		if (p_impl->m_trapped == EnemyTrappedState::Killed) Kill();
	}

	double EnLion::OrderPriority() const
	{
		return CharaOrderPriority(p_impl->m_pos);
	}

	bool EnLion::SendDamageCollider(const ItemAttackerAffair& attacker, const RectF& collider)
	{
		if (not IsEnemyCollided(*p_impl, collider)) return false;
		PerformEnemyDestroyed(attacker, *p_impl);
		Kill();
		return true;
	}

	const CharaPosition& EnLion::Pos() const
	{
		return p_impl->m_pos;
	}
}
