#pragma once
#include "Play/Forward.h"
#include "Play/Map/MapGrid.h"
#include "Util/CoroUtil.h"
#include "Util/Dir4.h"

namespace Play
{
	class CharaVec2 : public Vec2
	{
	public:
		using Vec2::Vec2;
		CharaVec2(Vec2 v): Vector2D<double>(v.x, v.y) { return; }

		Point MapPoint() const
		{
			return this->asPoint() / CellPx_24;
			// return (this->asPoint() + Point{CellPx_24 / 2, CellPx_24 / 2}) / CellPx_24;
		}
	};

	struct CharaPosition
	{
		CharaVec2 actualPos;
		Vec2 viewPos;

		void SetPos(const Vec2& pos)
		{
			actualPos = pos;
			viewPos = pos;
		}
	};

	class AnimTimer
	{
	public:
		void Reset(double t = 0) { m_timer = t; }
		void Tick(double dt = Scene::DeltaTime()) { m_timer += dt; }
		int SliceFrames(uint64 milliInterval, const Array<int>& pattern) const;
		int SliceFrames(uint64 milliInterval, uint32 numFrames) const;

	private:
		double m_timer{};
	};

	bool CanMoveTo(const MapGrid& map, const CharaVec2& currentActualPos, Dir4Type dir);

	void ProcessMoveCharaPos(
		YieldExtended& yield, ActorBase& self, CharaPosition& pos, const Vec2& nextPos, double moveDuration);

	Vec2 GetInitialPos(const MapGrid& map);

	double CharaOrderPriority(const CharaPosition& pos);
}
