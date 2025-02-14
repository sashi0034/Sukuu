﻿#include "stdafx.h"
#include "EnSlimeCat.h"

#include "Assets.generated.h"
#include "EnemyUtil.h"
#include "Play/PlayCore.h"
#include "Play/Chara/CharaUtil.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	constexpr Rect catRect{0, 0, 24, 24};
}

struct Play::EnSlimeCat::Impl : EnemyTransform
{
	bool m_prime{};
	bool m_doingLostPenalty{};
	int m_tireCount{};
	int m_tirePenalty{};

	void Update()
	{
		m_animTimer.Tick();

		// プレイヤーとの当たり判定
		CheckSendEnemyCollide(
			PlayCore::Instance().GetPlayer(), *this, m_prime ? EnemyKind::SlimeCat_prime : EnemyKind::SlimeCat);

		// 吹き出し描画
		const AssetNameView emotion = [&]()
		{
			if (m_trapped == EnemyTrappedState::Captured) return U"😵";
			if (m_playerTracker.IsTracking()) return U"😎";
			if (m_doingLostPenalty) return U"🤔";
			return U"";
		}();
		DrawEnemyBasically(*this, emotion);
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

	void StartTutorial(ActorBase& self, Dir4Type dir)
	{
		StartCoro(self, [this, self, dir](YieldExtended yield) mutable
		{
			while (true)
			{
				m_dir = dir;
				const double moveDuration = GetTomlParameter<double>(U"play.en_slime_cat.move_duration");
				auto&& playScene = PlayCore::Instance();
				if (not CanEnemyMoveTo(playScene.GetMap(), playScene.GetGimmick(), m_pos.actualPos, dir)) break;
				ProcessMoveCharaPos(yield, self, m_pos, m_pos.actualPos.moveBy(dir.ToXY() * CellPx_24), moveDuration);
			}
		});
	}

	Vec2 GetDrawPos() const override
	{
		return m_pos.viewPos.movedBy(m_animOffset + GetCharacterCellPadding(catRect.size));
	}

	TextureRegion GetTexture() const override
	{
		auto&& sheet = TextureAsset(AssetImages::punicat_24x24);
		const int interval = GetTomlParameter<int>(U"play.en_slime_cat.anim_interval");
		const int primeOffset = m_prime ? 4 : 0;

		switch (m_dir.GetIndex())
		{
		case Dir4Type::Right:
			return sheet(catRect.movedBy(catRect.size * Point{primeOffset + m_animTimer.SliceFrames(interval, 4), 3}));
		case Dir4Type::Up:
			return sheet(catRect.movedBy(catRect.size * Point{primeOffset + m_animTimer.SliceFrames(interval, 4), 2}));
		case Dir4Type::Left:
			return sheet(catRect.movedBy(catRect.size * Point{primeOffset + m_animTimer.SliceFrames(interval, 4), 1}));
		case Dir4Type::Down:
			return sheet(catRect.movedBy(catRect.size * Point{primeOffset + m_animTimer.SliceFrames(interval, 4), 0}));
		default: ;
			return {};
		}
	}

private:
	void checkFollowPlayer(YieldExtended& yield, const Point currentPoint)
	{
		m_playerTracker.Track(
			m_dir,
			currentPoint,
			GetTomlParameter<int>(U"play.en_slime_cat.max_player_concern"),
			[&]()
			{
				m_doingLostPenalty = true;
				yield.WaitForTime(GetTomlParameter<double>(U"play.en_slime_cat.lost_player_penalty"));
				m_doingLostPenalty = false;
			});
	}

	void flowchartLoop(YieldExtended& yield, ActorView self)
	{
		yield();
		const bool preRandomBool = Random(0, 1);
		int proceededCount = 0;
		while (true)
		{
			auto&& map = PlayCore::Instance().GetMap();
			auto&& gimmick = PlayCore::Instance().GetGimmick();
			const TerrainKind currentTerrain = GetTerrainAt(map, m_pos.actualPos);
			const auto currentPoint = m_pos.actualPos.MapPoint();

			// ギミック確認
			CheckEnemyTrappingGimmick(yield, self, currentPoint, *this);

			// プレイヤー追跡チェック
			checkFollowPlayer(yield, currentPoint);

			// 曲がるかチェック
			if (not m_playerTracker.IsTracking()
				&& checkTurn(map, gimmick, currentTerrain, proceededCount, preRandomBool))
				break;

			// 進行可能方向に向く
			if (FaceEnemyMovableDir(m_dir, m_pos, map, gimmick, preRandomBool) == false) return;

			// 進路方向に進む
			auto nextPos = m_pos.actualPos + m_dir.ToXY() * CellPx_24;
			const double moveDuration = GetTomlParameter<double>(U"play.en_slime_cat.move_duration");
			const double primeSpeed = m_prime
				                          ? (proceededCount % 4 == 0 ? 0.7 : 1.3)
				                          : 1.0;
			const double tireRate = (m_tirePenalty > 0 ? 2.0 : 1.0);
			ProcessMoveCharaPos(yield, self, m_pos, nextPos, moveDuration * tireRate / primeSpeed);
			proceededCount++;

			// 疲れを表現
			if (m_tirePenalty > 0)
			{
				m_tirePenalty--;
			}
			else
			{
				if (not m_prime) m_tireCount--;
			}
			if (m_tireCount < 0)
			{
				m_tireCount = GetTomlParameter<int>(U"play.en_slime_cat.tire_count");
				m_tirePenalty = Random(0, GetTomlParameter<int>(U"play.en_slime_cat.tire_penalty"));
			}
		}
	}

	bool checkTurn(const MapGrid& map, const GimmickGrid& gimmick,
	               TerrainKind currentTerrain, int proceededCount, bool leftPriority)
	{
		if (proceededCount == 0) return false;

		// 部屋から出れるかチェック
		if (currentTerrain == TerrainKind::Floor &&
			RandomBool(GetTomlParameter<double>(U"play.en_slime_cat.floor_turn_pr")))
		{
			return RotateEnemyDirFacingPathway(m_dir, m_pos, map, leftPriority);
		}

		// 通路を曲がるかチェック
		if (currentTerrain == TerrainKind::Pathway &&
			RandomBool(GetTomlParameter<double>(U"play.en_slime_cat.pathway_turn_pr")))
		{
			return RotateEnemyDirFacingMovable(m_dir, m_pos, map, gimmick, leftPriority);
		}
		return false;
	}
};

namespace Play
{
	EnSlimeCat::EnSlimeCat() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void EnSlimeCat::Init()
	{
		p_impl->m_pos.SetPos(GetInitialPos(PlayCore::Instance().GetMap()));
		p_impl->StartFlowchart(*this);
	}

	void EnSlimeCat::InitTutorial(const CharaVec2& pos, Dir4Type dir)
	{
		p_impl->m_pos.SetPos(pos);
		p_impl->StartTutorial(*this, dir);
	}

	void EnSlimeCat::BecomePrime()
	{
		p_impl->m_prime = true;
	}

	void EnSlimeCat::Update()
	{
		ActorBase::Update();
		p_impl->Update();
		if (p_impl->m_trapped == EnemyTrappedState::Killed) Kill();
	}

	double EnSlimeCat::OrderPriority() const
	{
		return CharaOrderPriority(p_impl->m_pos);
	}

	bool EnSlimeCat::SendDamageCollider(const ItemAttackerAffair& attacker, const RectF& collider)
	{
		if (not IsEnemyCollided(*p_impl, collider)) return false;
		PerformEnemyDestroyed(attacker, *p_impl);
		Kill();
		return true;
	}

	const CharaPosition& EnSlimeCat::Pos() const
	{
		return p_impl->m_pos;
	}
}
