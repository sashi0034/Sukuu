﻿#include "stdafx.h"
#include "EnemyUtil.h"

#include "Assets.generated.h"
#include "Play/PlayCore.h"
#include "Play/Effect/FragmentTextureEffect.h"
#include "Play/Other/PlayPenaltyBonus.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	using namespace Play;

	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.enemy." + key);
	}

	// マグネット上で小刻みに一回転をする演出
	void performMagnetTrap(YieldExtended& yield, ActorView self, EnemyTransform& transform)
	{
		transform.m_dir = transform.m_dir.RotatedL();

		double theta = 0.0;
		const double penalty = getToml<double>(U"magnet_penalty");
		auto easing = AnimateEasing<EaseInLinear>(self, &theta, Math::TwoPi, penalty);

		const Vec2 viewStart = transform.m_pos.actualPos;
		const Vec2 viewEnd = transform.m_pos.actualPos.movedBy(transform.m_dir.ToXY() * 8.0);
		const Vec2 viewCenter = (viewStart + viewEnd) / 2.0;
		const double viewRadius = viewStart.distanceFrom(viewEnd) / 2.0;
		const double startTheta = (viewStart - viewCenter).getAngle();

		while (true)
		{
			transform.m_pos.viewPos = viewCenter + Circular(viewRadius, startTheta + theta);
			if (easing.IsDead()) break;
			yield();
		}
	}
}

namespace Play
{
	bool CanEnemyMoveTo(const MapGrid& map, const GimmickGrid& gimmick, const CharaVec2& currentActualPos, Dir4Type dir)
	{
		const auto nextPoint = currentActualPos.MapPoint() + dir.ToXY().asPoint();
		if (gimmick.inBounds(nextPoint) && gimmick[nextPoint] == GimmickKind::Installed_Grave) return false;
		return CanMovePointAt(map, nextPoint);
	}

	bool CheckSendEnemyCollide(Player& player, const EnemyTransform& transform, EnemyKind enemy)
	{
		if (not transform.m_collideEnabled) return false;
		auto&& pos = transform.m_pos;
		if (player.DistField()[pos.actualPos.MapPoint()].distance != PlayerDistanceInfinity)
		{
			return player.SendEnemyCollide({pos.actualPos, {CellPx_24, CellPx_24}}, enemy);
		}
		return false;
	}

	void DrawEnemyBasically(const IEnemyInternal& enemy, AssetNameView emotion)
	{
		const auto drawingPos = enemy.GetDrawPos();
		(void)enemy.GetTexture().draw(drawingPos);
		if (not emotion.empty()) DrawCharaEmotion(drawingPos, emotion);
	}

	bool IsEnemyCollided(const EnemyTransform& transform, const RectF& collider)
	{
		if (not transform.m_collideEnabled) return false;
		auto&& pos = transform.m_pos;
		return RectF{pos.actualPos, Vec2{CellPx_24, CellPx_24}}.intersects(collider);
	}

	void PerformEnemyDestroyed(const ItemAttackerAffair& attacker, const EnemyTransform& enemy)
	{
		const auto drawingPos = enemy.GetDrawPos();
		const auto texture = enemy.GetTexture();
		auto&& playScene = PlayCore::Instance();
		playScene.RequestHitstopping(0.5);
		playScene.FgEffect().add(EmitFragmentTextureEffect(
			drawingPos.movedBy(texture.size / 2),
			texture,
			Palette::Crimson,
			96 + attacker.AttackedCount() * 12));
		RelayTimeHealAmount(enemy.m_pos, GetEnemyKilledBonusHeal(attacker.AttackedCount()));
	}

	bool FaceEnemyMovableDir(
		Dir4Type& dir, const CharaPosition& pos, const MapGrid& map, const GimmickGrid& gimmick, bool leftPriority)
	{
		// 直線方向に進めるかチェック
		if (CanEnemyMoveTo(map, gimmick, pos.actualPos, dir) == false)
		{
			// 直進できないなら、曲がれるかチェック
			if (CanEnemyMoveTo(map, gimmick, pos.actualPos, dir.Rotated(leftPriority)))
			{
				dir = dir.Rotated(leftPriority);
			}
			else if (CanEnemyMoveTo(map, gimmick, pos.actualPos, dir.Rotated(not leftPriority)))
			{
				dir = dir.Rotated(not leftPriority);
			}
			else
			{
				dir = dir.Reversed();
				return false;
			}
		}
		return true;
	}

	bool RotateEnemyDirFacingPathway(Dir4Type& dir, const CharaPosition& pos, const MapGrid& map, bool leftPriority)
	{
		if (GetTerrainFor(map, pos.actualPos, dir.Rotated(leftPriority)) == TerrainKind::Pathway)
		{
			dir = dir.Rotated(leftPriority);
			return true;
		}
		if (GetTerrainFor(map, pos.actualPos, dir.Rotated(not leftPriority)) == TerrainKind::Pathway)
		{
			dir = dir.Rotated(not leftPriority);
			return true;
		}
		return false;
	}

	bool RotateEnemyDirFacingMovable(
		Dir4Type& dir, const CharaPosition& pos, const MapGrid& map, const GimmickGrid& gimmick, bool leftPriority)
	{
		if (CanEnemyMoveTo(map, gimmick, pos.actualPos, dir.Rotated(leftPriority)))
		{
			dir = dir.Rotated(leftPriority);
			return true;
		}
		if (CanEnemyMoveTo(map, gimmick, pos.actualPos, dir.Rotated(not leftPriority)))
		{
			dir = dir.Rotated(not leftPriority);
			return true;
		}
		return false;
	}

	void EnemyPlayerTracker::Track(
		Dir4Type& direction,
		const Point& currentPoint,
		int maxConcern,
		const std::function<void()>& onLostPlayer)
	{
		auto&& player = PlayCore::Instance().GetPlayer();
		auto&& playerDf = player.DistField();
		const int currentDist = playerDf[currentPoint].distance;
		const auto nextPoint = currentPoint + direction.ToXY().asPoint();
		const auto nextDist = playerDf[nextPoint].distance;

		const bool isGettingAway = nextDist > currentDist || currentDist == PlayerDistanceInfinity;
		const bool isPlayerImmortal = player.IsImmortal();
		if ((isGettingAway || isPlayerImmortal) && m_concern > 0)
		{
			// 追跡中だけど、プレイヤーから遠ざかっている
			m_concern--;

			// プレイヤー見失った
			if (m_concern <= 0)
			{
				// ペナルティ
				if (onLostPlayer) onLostPlayer();
			}
		}

		// プレイヤーが無敵のときは、追跡しない
		if (isPlayerImmortal) return;

		auto&& scene = PlayCore::Instance();
		const bool isMazeMap = scene.GetMap().Category() == MapCategory::Maze;
		const auto resetTracking = [&]()
		{
			if (m_concern == 0) AudioAsset(AssetSes::enemy_find).playOneShot();
			m_concern = isMazeMap ? maxConcern * 5 : maxConcern;
		};
		const auto playerPoint = scene.GetPlayer().CurrentPoint();
		const bool isInPathway =
			scene.GetMap().Data()[currentPoint].kind == TerrainKind::Pathway
			|| scene.GetMap().Data()[playerPoint].kind == TerrainKind::Pathway;
		if ((not isInPathway) || isMazeMap)
		{
			// 部屋の中 (迷路は常に部屋扱い)
			if (nextDist < currentDist) resetTracking();
		}
		else
		{
			// 通路の中
			if ((playerDf[currentPoint].directStraight || playerDf[nextPoint].directStraight) && nextDist < currentDist)
				resetTracking();
		}

		if (m_concern == 0) return;
		// 以下、プレイヤー追跡中

		// このまま正面に進むとプレイヤーから離れてしまうとき、方向転換
		if (nextDist > currentDist)
		{
			if (playerDf[currentPoint + direction.RotatedL().ToXY().asPoint()].distance < currentDist)
			{
				direction = direction.RotatedL();
			}
			else if (playerDf[currentPoint + direction.RotatedR().ToXY().asPoint()].distance < currentDist)
			{
				direction = direction.RotatedR();
			}
			else if (playerDf[currentPoint + direction.Reversed().ToXY().asPoint()].distance < currentDist)
			{
				direction = direction.Reversed();
			}
		}
	}

	bool CheckEnemyTrappingGimmick(
		YieldExtended& yield, ActorView self, const Point& currentPoint, EnemyTransform& transform)
	{
		auto&& gimmick = PlayCore::Instance().GetGimmick();
		switch (gimmick[currentPoint])
		{
		case GimmickKind::Installed_Mine: {
			// 地雷を踏んだ
			gimmick[currentPoint] = GimmickKind::None;
			// TODO: 爆発エフェクト
			PerformEnemyDestroyed(ItemAttackerAffair{ConsumableItem::Mine}, transform);
			transform.m_trapped = EnemyTrappedState::Killed;
			yield();
			return true;
		}
		case GimmickKind::Installed_Magnet:
			// マグネットに引っかかった
			transform.m_trapped = EnemyTrappedState::Captured;
			while (gimmick[currentPoint] == GimmickKind::Installed_Magnet)
			{
				performMagnetTrap(yield, self, transform);
			}

			transform.m_trapped = EnemyTrappedState::Normal;
			return true;
		case GimmickKind::Arrow_right: [[fallthrough]];
		case GimmickKind::Arrow_up: [[fallthrough]];
		case GimmickKind::Arrow_left: [[fallthrough]];
		case GimmickKind::Arrow_down: {
			// 矢印
			transform.m_collideEnabled = false;
			const Vec2 nextPos = GetArrowWarpPoint(
				PlayCore::Instance().GetMap(),
				PlayCore::Instance().GetGimmick(),
				transform.m_pos.actualPos.MapPoint()) * CellPx_24;
			ProcessArrowWarpCharaPos(yield, self, transform.m_pos, transform.m_animOffset, nextPos);
			transform.m_collideEnabled = true;
			break;
		}
		default:
			break;
		}
		return false;
	}
}
