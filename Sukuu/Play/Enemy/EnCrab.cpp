#include "stdafx.h"
#include "EnCrab.h"

#include "Assets.generated.h"
#include "EnemyUtil.h"
#include "Play/PlayCore.h"
#include "Play/Chara/CharaUtil.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.en_crab." + key);
	}

	constexpr Rect spriteRect{0, 0, 24, 24};
}

struct Play::EnCrab::Impl : EnemyTransform
{
	bool m_doingLostPenalty = false;

	void Update()
	{
		m_animTimer.Tick();

		// プレイヤーとの当たり判定
		CheckSendEnemyCollide(PlayCore::Instance().GetPlayer(), *this, EnemyKind::Crab);

		const AssetNameView emotion = [&]()
		{
			if (m_trapped == EnemyTrappedState::Captured) return U"😵";
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
			TextureAsset(AssetImages::rust_ferris_24x24)(
				spriteRect.movedBy(m_animTimer.SliceFrames(150, 6) * spriteRect.w, 0));
		return t;
	}

private:
	void flowchartLoop(YieldExtended& yield, ActorView self)
	{
		yield();

		// 移動中
		const bool leftPriority = RandomBool(0.5);
		while (true)
		{
			auto&& map = PlayCore::Instance().GetMap();
			auto&& gimmick = PlayCore::Instance().GetGimmick();
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

			// 曲がるかチェック
			if (not m_playerTracker.IsTracking())
				checkTurn(map, gimmick, currentTerrain, leftPriority);

			// 進行可能方向に向く
			if (FaceEnemyMovableDir(m_dir, m_pos, map, gimmick, leftPriority) == false) break;

			// 移動開始
			auto nextPos = m_pos.actualPos + m_dir.ToXY() * CellPx_24;
			const double moveDuration = m_dir.ToXY().x != 0
				                            ? getToml<double>(U"move_duration_x")
				                            : getToml<double>(U"move_duration_y");

			if (m_dir.ToXY().y != 0 || not CanMoveTo(map, nextPos, m_dir))
				// 行き止まりや縦移動の時はイージングをかける
				ProcessMoveCharaPos<EaseOutBack>(yield, self, m_pos, nextPos, moveDuration);
			else
				ProcessMoveCharaPos(yield, self, m_pos, nextPos, moveDuration);
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
	EnCrab::EnCrab() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void EnCrab::Init()
	{
		p_impl->m_pos.SetPos(GetInitialPos(PlayCore::Instance().GetMap()));
		p_impl->StartFlowchart(*this);
	}

	void EnCrab::Update()
	{
		ActorBase::Update();
		p_impl->Update();
		if (p_impl->m_trapped == EnemyTrappedState::Killed) Kill();
	}

	double EnCrab::OrderPriority() const
	{
		return CharaOrderPriority(p_impl->m_pos);
	}

	bool EnCrab::SendDamageCollider(const ItemAttackerAffair& attacker, const RectF& collider)
	{
		if (not IsEnemyCollided(*p_impl, collider)) return false;
		PerformEnemyDestroyed(attacker, *p_impl);
		Kill();
		return true;
	}

	const CharaPosition& EnCrab::Pos() const
	{
		return p_impl->m_pos;
	}
}
