#pragma once
#include "Play/Forward.h"
#include "Play/Gimmick/GimmickGrid.h"
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
			return (this->asPoint() + Point(CellPx_24, CellPx_24) / 2) / CellPx_24;
			// return (this->asPoint() + Point{CellPx_24 / 2, CellPx_24 / 2}) / CellPx_24;
		}
	};

	struct CharaPosition
	{
		// 実座標
		CharaVec2 actualPos;
		// ビューの座標は、できるだけ実座標に近づけるがフレーム間隔による浮動小数点誤差で数ピクセルずれる可能性がある
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
		AnimTimer() = default;
		AnimTimer(double time) { m_timer = time; }

		void Reset(double t = 0) { m_timer = t; }
		void Tick(double dt = GetDeltaTime()) { m_timer += dt; }
		int SliceFrames(uint64 milliInterval, const Array<int>& pattern) const;
		int SliceFrames(uint64 milliInterval, uint32 numFrames) const;
		int SliceFramesWithOffset(uint64 milliInterval, uint32 numFrames, uint32 offset) const;
		double Time() const { return m_timer; }

	private:
		double m_timer{};
	};

	TerrainKind GetTerrainAt(const MapGrid& map, const CharaVec2& pos);
	TerrainKind GetTerrainFor(const MapGrid& map, const CharaVec2& pos, Dir4Type dir);
	bool CanMoveTo(const MapGrid& map, const CharaVec2& currentActualPos, Dir4Type dir);
	bool CanMovePointTo(const MapGrid& map, const Point& point, Dir4Type dir);
	bool CanMovePointAt(const MapGrid& map, const Point& point);

	Point GetArrowWarpPoint(const MapGrid& map, const GimmickGrid& gimmickGrid, const Point& point);

	template <double easing(double) = EaseInLinear>
	void ProcessMoveCharaPos(
		YieldExtended& yield, ActorView self, CharaPosition& pos, const Vec2& nextPos, double moveDuration);
	template void ProcessMoveCharaPos<EaseInLinear>(
		YieldExtended& yield, ActorView self, CharaPosition& pos, const Vec2& nextPos, double moveDuration);
	template void ProcessMoveCharaPos<EaseOutCirc>(
		YieldExtended& yield, ActorView self, CharaPosition& pos, const Vec2& nextPos, double moveDuration);
	template void ProcessMoveCharaPos<EaseOutBack>(
		YieldExtended& yield, ActorView self, CharaPosition& pos, const Vec2& nextPos, double moveDuration);
	template void ProcessMoveCharaPos<EaseInOutBack>(
		YieldExtended& yield, ActorView self, CharaPosition& pos, const Vec2& nextPos, double moveDuration);

	void ProcessArrowWarpCharaPos(
		YieldExtended& yield, ActorView self, CharaPosition& pos, Vec2& jumpOffset, const Vec2& nextPos);

	void ProcessDemiArrowWarpCharaPos(
		YieldExtended& yield, ActorView self, CharaPosition& pos, Vec2& jumpOffset, const Vec2& nextPos);

	Vec2 GetInitialPos(const MapGrid& map, bool allowPathway = false);

	double CharaOrderPriority(const CharaPosition& pos);

	void DrawCharaEmotion(const Vec2& drawingPos, AssetNameView emoji);

	SINGLETON_SIDEEFFECT void RelayTimeDamageAmount(const CharaPosition& pos, int amount, bool isEnemyDamage);
	SINGLETON_SIDEEFFECT void RelayTimeHealAmount(const CharaPosition& pos, int amount);
}
