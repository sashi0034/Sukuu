#include "stdafx.h"
#include "EnKnight.h"

#include "Assets.generated.h"
#include "EnemyUtil.h"
#include "Play/PlayCore.h"
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

struct Play::EnKnight::Impl : EnemyTransform
{
	bool m_prime{};
	bool m_sleeping = true;
	int m_awakeCount{};
	bool m_doingLostPenalty = false;

	void Update()
	{
		m_animTimer.Tick();

		// プレイヤーとの当たり判定
		CheckSendEnemyCollide(PlayCore::Instance().GetPlayer(), *this, EnemyKind::Knight);

		const AssetNameView emotion = [&]()
		{
			if (m_trapped == EnemyTrappedState::Captured) return U"😵";
			if (m_sleeping) return U"😴";
			if (m_playerTracker.IsTracking()) return U"😎";
			if (m_doingLostPenalty) return U"🤔";
			return U"";
		}();
		DrawEnemyBasically(*this, emotion);
	}

	Vec2 GetDrawPos() const override
	{
		return m_pos.viewPos.movedBy(m_animOffset + GetCharacterCellPadding(knightRect.size));
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
		const int primeOffset = m_prime ? knightRect.h : 0;
		if (m_sleeping) return sheet(knightRect.movedBy(knightRect.w * 4, primeOffset));
		auto&& t =
			sheet(knightRect.movedBy(m_animTimer.SliceFrames(250, 4) * knightRect.w, primeOffset));
		return m_dir == Dir4::Left || m_dir == Dir4::Up ? t.mirrored() : t;
	}

private:
	void flowchartLoop(YieldExtended& yield, ActorView self)
	{
		if (m_prime &&
			m_awakeCount >= 4 && not m_playerTracker.IsTracking())
		{
			// プライムは途中でスリープする可能性あり
			m_awakeCount = 0;
			m_sleeping = true;
		}

		// スリープ中
		double sleepingTime{};
		while (m_sleeping)
		{
			yield();
			auto&& playerDf = PlayCore::Instance().GetPlayer().DistField();
			// プレイヤーが近くに来たら起動
			const bool nearPlayer = playerDf[m_pos.actualPos.MapPoint()].distance < getToml<int>(U"sleeping_sensing");
			// プライムは時間経過でも起動
			sleepingTime += GetDeltaTime();
			const bool primeAwake = m_prime ? sleepingTime > 5.0 : false;
			if (nearPlayer || primeAwake)
			{
				m_sleeping = false;
			}
		}
		m_awakeCount++;

		// 移動中
		int proceededCount{};
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
			const double primeSpeed = m_prime ? 1.1 : 1.0;
			ProcessMoveCharaPos(yield, self, m_pos, nextPos, moveDuration / primeSpeed);
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
		p_impl->m_pos.SetPos(GetInitialPos(PlayCore::Instance().GetMap()));
		p_impl->StartFlowchart(*this);
	}

	void EnKnight::InitTutorial(const CharaVec2& pos, Dir4Type dir)
	{
		p_impl->m_pos.SetPos(pos);
		p_impl->m_sleeping = false;
		p_impl->m_dir = dir;
	}

	void EnKnight::BecomePrime()
	{
		p_impl->m_prime = true;
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

	bool EnKnight::SendDamageCollider(const ItemAttackerAffair& attacker, const RectF& collider)
	{
		if (not IsEnemyCollided(*p_impl, collider)) return false;
		PerformEnemyDestroyed(attacker, *p_impl);
		Kill();
		return true;
	}

	const CharaPosition& EnKnight::Pos() const
	{
		return p_impl->m_pos;
	}
}
