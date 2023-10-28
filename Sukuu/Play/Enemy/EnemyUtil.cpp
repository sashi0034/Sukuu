#include "stdafx.h"
#include "EnemyUtil.h"

#include "Play/PlayScene.h"
#include "Play/Effect/FragmentTextureEffect.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.enemy." + key);
	}
}

namespace Play
{
	void CheckSendEnemyCollide(Player& player, CharaPosition& pos, EnemyKind enemy)
	{
		if (player.DistField()[pos.actualPos.MapPoint()].distance != PlayerDistanceInfinity)
		{
			player.SendEnemyCollide({pos.actualPos, {CellPx_24, CellPx_24}}, enemy);
		}
	}

	bool IsEnemyCollided(const CharaPosition& pos, const RectF& collider)
	{
		return RectF{pos.actualPos, Vec2{CellPx_24, CellPx_24}}.intersects(collider);
	}

	static void performEnemyDestroyed(const Vec2& drawingPos, const TextureRegion& texture)
	{
		auto&& playScene = PlayScene::Instance();
		playScene.RequestHitstopping(0.5);
		playScene.FgEffect().add(EmitFragmentTextureEffect(
			drawingPos.movedBy(texture.size / 2),
			texture,
			Palette::Crimson,
			96));
	}

	void PerformEnemyDestroyed(const IEnemyInternal& enemy)
	{
		performEnemyDestroyed(enemy.GetDrawPos(), enemy.GetTexture());
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
		auto&& player = PlayScene::Instance().GetPlayer();
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

		const auto resetTracking = [&]()
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

	bool CheckEnemyTrappingGimmick(const Point& currentPoint, const std::function<void()>& onKilled)
	{
		auto&& gimmick = PlayScene::Instance().GetGimmick();
		switch (gimmick[currentPoint])
		{
		case GimmickKind::Installed_Mine:
			// 地雷を踏んだ
			onKilled();
			gimmick[currentPoint] = GimmickKind::None;
			return true;
		default:
			break;
		}
		return false;
	}

	bool CheckEnemyTrappingGimmick(
		YieldExtended& yield,
		const Point& currentPoint,
		const IEnemyInternal& enemy,
		Dir4Type& dir,
		EnemyTrappedState& trappedState)
	{
		auto&& gimmick = PlayScene::Instance().GetGimmick();
		switch (gimmick[currentPoint])
		{
		case GimmickKind::Installed_Mine: {
			// 地雷を踏んだ
			gimmick[currentPoint] = GimmickKind::None;
			// TODO: 爆発エフェクト
			PerformEnemyDestroyed(enemy);
			trappedState = EnemyTrappedState::Killed;
			yield();
			return true;
		}
		case GimmickKind::Installed_Magnet:
			// マグネットに引っかかった
			trappedState = EnemyTrappedState::Captured;
			while (gimmick[currentPoint] == GimmickKind::Installed_Magnet)
			{
				yield.WaitForTime(getToml<double>(U"magnet_penalty"));
				dir = dir.Rotated(RandomBool(0.5));
			}
			trappedState = EnemyTrappedState::Normal;
			return true;
		default:
			break;
		}
		return false;
	}
}
