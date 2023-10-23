#include "stdafx.h"
#include "CharaUtil.h"

#include "Util/EasingAnimation.h"

namespace Play
{
	int AnimTimer::SliceFrames(uint64 milliInterval, const Array<int>& pattern) const
	{
		const uint64 t = m_timer * 1000;
		const int index = (t % (milliInterval * pattern.size())) / milliInterval;
		return pattern[index];
	}

	int AnimTimer::SliceFrames(uint64 milliInterval, uint32 numFrames) const
	{
		const uint64 t = m_timer * 1000;
		return (t % (milliInterval * numFrames)) / milliInterval;
	}

	TerrainKind GetTerrainAt(const MapGrid& map, const CharaVec2& pos)
	{
		const Point p = pos.MapPoint();
		if (map.Data().inBounds(p) == false) return TerrainKind::None;
		return map.Data()[p].kind;
	}

	TerrainKind GetTerrainFor(const MapGrid& map, const CharaVec2& pos, Dir4Type dir)
	{
		const Point p = pos.MapPoint() + dir.ToXY().asPoint();
		if (map.Data().inBounds(p) == false) return TerrainKind::None;
		return map.Data()[p].kind;
	}

	bool canMoveAtInternal(const MapGrid& map, const Point next)
	{
		if (map.Data().inBounds(next) == false) return false;
		return map.At(next).kind != TerrainKind::Wall;
	}

	bool CanMoveTo(const MapGrid& map, const CharaVec2& currentActualPos, Dir4Type dir)
	{
		const Point next = currentActualPos.MapPoint() + dir.ToXY().asPoint();
		return canMoveAtInternal(map, next);
	}

	bool CanMovePointTo(const MapGrid& map, const Point& point, Dir4Type dir)
	{
		const Point next = point + dir.ToXY().asPoint();
		return canMoveAtInternal(map, next);
	}

	bool CanMovePointAt(const MapGrid& map, const Point& point)
	{
		return canMoveAtInternal(map, point);
	}

	void ProcessMoveCharaPos(
		YieldExtended& yield, ActorBase& self, CharaPosition& pos, const Vec2& nextPos, double moveDuration)
	{
		// 実座標を移動
		Util::AnimateEasing<EaseInLinear, EaseOption::Default>(
			self, &pos.actualPos, CharaVec2(nextPos),
			moveDuration);
		// ビュー座標を移動し、これを待機
		yield.WaitForDead(
			Util::AnimateEasing<EaseInLinear, EaseOption::None>(
				self, &pos.viewPos, nextPos,
				moveDuration)
		);
	}

	Vec2 GetInitialPos(const MapGrid& map)
	{
		return map.Rooms().RandomRoomPoint() * CellPx_24;
	}

	double CharaOrderPriority(const CharaPosition& pos)
	{
		return 1000.0 + (pos.viewPos.y / (CellPx_24 * 1024));
	}
}
