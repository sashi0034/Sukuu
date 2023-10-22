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

	bool CanMoveTo(const MapGrid& map, const CharaVec2& current, Dir4Type dir)
	{
		const Point next = current.MapPoint() + dir.ToXY().asPoint();
		if (map.Data().inBounds(next) == false) return false;
		return map.At(next).kind != TerrainKind::Wall;
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
}
