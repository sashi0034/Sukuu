#include "stdafx.h"
#include "EnemyUtil.h"

#include "Play/PlayScene.h"
#include "Play/Effect/FragmentTextureEffect.h"

namespace Play
{
	void CheckSendEnemyCollide(Player& player, CharaPosition& pos)
	{
		if (player.DistField()[pos.actualPos.MapPoint()].distance != PlayerDistanceInfinity)
		{
			player.SendEnemyCollide({pos.actualPos, {CellPx_24, CellPx_24}});
		}
	}

	bool IsEnemyCollided(const CharaPosition& pos, const RectF& collider)
	{
		return RectF{pos.actualPos, Vec2{CellPx_24, CellPx_24}}.intersects(collider);
	}

	void PerformEnemyDestroyed(const Vec2& drawingPos, const TextureRegion& texture)
	{
		auto&& playScene = PlayScene::Instance();
		playScene.RequestHitstopping(0.5);
		playScene.FgEffect().add(EmitFragmentTextureEffect(
			drawingPos.movedBy(texture.size / 2),
			texture,
			Palette::Crimson,
			96));
	}

	bool FaceEnemyMovableDir(Dir4Type& dir, const CharaPosition& pos, const MapGrid& map, bool leftPriority)
	{
		// 直線方向に進めるかチェック
		if (CanMoveTo(map, pos.actualPos, dir) == false)
		{
			// 直進できないなら、曲がれるかチェック
			if (CanMoveTo(map, pos.actualPos, dir.Rotated(leftPriority)))
			{
				dir = dir.Rotated(leftPriority);
			}
			else if (CanMoveTo(map, pos.actualPos, dir.Rotated(not leftPriority)))
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

	bool RotateEnemyDirFacingMovable(Dir4Type& dir, const CharaPosition& pos, const MapGrid& map, bool leftPriority)
	{
		if (CanMoveTo(map, pos.actualPos, dir.Rotated(leftPriority)))
		{
			dir = dir.Rotated(leftPriority);
			return true;
		}
		if (CanMoveTo(map, pos.actualPos, dir.Rotated(not leftPriority)))
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
		auto&& playerDf = PlayScene::Instance().GetPlayer().DistField();
		const int currentDist = playerDf[currentPoint].distance;
		const auto nextPoint = currentPoint + direction.ToXY().asPoint();
		const auto nextDist = playerDf[nextPoint].distance;

		if ((nextDist > currentDist || currentDist == PlayerDistanceInfinity) && m_concern > 0)
		{
			// 追跡中だけど、プレイヤーから遠ざかっている
			m_concern--;

			// プレイヤー見失った
			if (m_concern == 0)
			{
				// ペナルティ
				if (onLostPlayer) onLostPlayer();
			}
		}

		auto resetTracking = [&]()
		{
			m_concern = maxConcern;
		};
		const auto playerPoint = PlayScene::Instance().GetPlayer().CurrentPoint();
		const bool isInPathway =
			PlayScene::Instance().GetMap().Data()[currentPoint].kind == TerrainKind::Pathway
			|| PlayScene::Instance().GetMap().Data()[playerPoint].kind == TerrainKind::Pathway;
		if (isInPathway)
		{
			// 通路の中
			if ((playerDf[currentPoint].directStraight || playerDf[nextPoint].directStraight) && nextDist <
				currentDist)
				resetTracking();
		}
		else
		{
			// 部屋の中
			if (nextDist < currentDist) resetTracking();
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
}
