#include "stdafx.h"
#include "EnKnight.h"

#include "Assets.generated.h"
#include "EnemyUtil.h"
#include "Play/PlayScene.h"
#include "Play/Chara/CharaUtil.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.en_knight." + key);
	}

	constexpr Rect knightRect{0, 0, 32, 32};
}

struct Play::EnKnight::Impl : IEnemyInternal
{
	EnemyTrappedState m_trapped{};
	CharaPosition m_pos{};
	Dir4Type m_dir{Dir4::Down};
	AnimTimer m_animTimer{};
	EnemyPlayerTracker m_playerTracker{};

	bool m_sleeping = true;
	bool m_doingLostPenalty = false;

	void Update()
	{
		// プレイヤーとの当たり判定
		CheckSendEnemyCollide(PlayScene::Instance().GetPlayer(), m_pos, EnemyKind::Knight);

		// アニメーション更新
		m_animTimer.Tick();
		const auto drawingPos = GetDrawPos();
		(void)GetTexture().draw(drawingPos);

		// 吹き出し描画
		const AssetNameView emotion = [&]()
		{
			if (m_trapped == EnemyTrappedState::Captured) return U"😬";
			if (m_sleeping) return U"😴";
			if (m_playerTracker.IsTracking()) return U"😎";
			if (m_doingLostPenalty) return U"🤔";
			return U"";
		}();

		if (not emotion.empty()) DrawCharaEmotion(drawingPos, emotion);
	}

	Vec2 GetDrawPos() const
	{
		return m_pos.viewPos.movedBy(GetCharacterCellPadding(knightRect.size));
	}

	void StartFlowchart(ActorBase& self)
	{
		StartCoro(self, [this, self](YieldExtended yield) mutable
		{
			while (true)
			{
				flowchartLoop(yield, self);
			}
		});
	}

	TextureRegion GetTexture() const override
	{
		auto&& sheet = TextureAsset(AssetImages::temple_knight_side_32x32);
		if (m_sleeping) return sheet(knightRect.movedBy(0, knightRect.h));
		auto&& t =
			TextureAsset(AssetImages::temple_knight_side_32x32)(
				knightRect.movedBy(m_animTimer.SliceFrames(250, 4) * knightRect.w, 0));
		return m_dir == Dir4::Left || m_dir == Dir4::Up ? t.mirrored() : t;
	}

private:
	void flowchartLoop(YieldExtended& yield, ActorView self)
	{
		// スリープ中
		while (m_sleeping)
		{
			yield();
			auto&& playerDf = PlayScene::Instance().GetPlayer().DistField();
			if (playerDf[m_pos.actualPos.MapPoint()].distance < getToml<int>(U"sleeping_sensing"))
			{
				// プレイヤーが近くに来たので起動
				m_sleeping = false;
			}
		}

		// 移動中
		int proceededCount{};
		while (true)
		{
			auto&& map = PlayScene::Instance().GetMap();
			auto&& gimmick = PlayScene::Instance().GetGimmick();
			const TerrainKind currentTerrain = GetTerrainAt(map, m_pos.actualPos);
			const auto currentPoint = m_pos.actualPos.MapPoint();

			// ギミック確認
			CheckEnemyTrappingGimmick(yield, currentPoint, *this, m_dir, m_trapped);

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
					proceededCount = 0;
				});

			// 曲がるかチェック
			if (not m_playerTracker.IsTracking() && checkTurn(map, gimmick, currentTerrain, RandomBool(0.5)))
				break;

			// 進行可能方向に向く
			if (FaceEnemyMovableDir(m_dir, m_pos, map, gimmick, RandomBool(0.5)) == false) break;

			// 移動
			auto nextPos = m_pos.actualPos + m_dir.ToXY() * CellPx_24;
			const double moveDuration = std::max(
				getToml<double>(U"move_duration") - proceededCount * proceededCount * getToml<double>(U"speedup"),
				getToml<double>(U"min_move_duration"));
			ProcessMoveCharaPos(yield, self, m_pos, nextPos, moveDuration);
			proceededCount++;
		}
	}

	bool checkTurn(const MapGrid& map, const GimmickGrid& gimmick, TerrainKind currentTerrain, bool leftPriority)
	{
		// 部屋から出れるかチェック
		if (currentTerrain == TerrainKind::Floor &&
			RandomBool(0.5))
		{
			return RotateEnemyDirFacingPathway(m_dir, m_pos, map, leftPriority);
		}

		// 通路を曲がるかチェック
		if (currentTerrain == TerrainKind::Pathway &&
			RandomBool(0.5))
		{
			return RotateEnemyDirFacingMovable(m_dir, m_pos, map, gimmick, leftPriority);
		}
		return false;
	}
};

namespace Play
{
	EnKnight::EnKnight() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void EnKnight::Init()
	{
		p_impl->m_pos.SetPos(GetInitialPos(PlayScene::Instance().GetMap()));

		p_impl->StartFlowchart(*this);
	}

	void EnKnight::Update()
	{
		ActorBase::Update();
		p_impl->Update();
		if (p_impl->m_trapped == EnemyTrappedState::Killed) Kill();
	}

	double EnKnight::OrderPriority() const
	{
		return CharaOrderPriority(p_impl->m_pos);
	}

	bool EnKnight::SendDamageCollider(ItemAttackerAffair& attacker, const RectF& collider)
	{
		if (not IsEnemyCollided(p_impl->m_pos, collider)) return false;
		attacker.IncAttacked();
		PerformEnemyDestroyed(*p_impl);
		Kill();
		return true;
	}

	const CharaPosition& EnKnight::Pos() const
	{
		return p_impl->m_pos;
	}
}
