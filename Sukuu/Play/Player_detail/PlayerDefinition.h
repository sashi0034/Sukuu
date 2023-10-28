#pragma once

#include "Play/Item/ConsumableItem.h"
#include "Play/Forward.h"

namespace Play
{
	constexpr int PlayerDistanceInfinity = INT32_MAX / 2;

	constexpr Rect PlayerCellRect{0, 0, 32, 32};

	struct PlayerPosRelation
	{
		int distance = PlayerDistanceInfinity;
		bool directStraight = false;
	};

	struct PlayerPersonalData
	{
		std::array<ConsumableItem, MaxItemPossession> items;
	};

	class PlayerDistField : public Grid<PlayerPosRelation>
	{
	public:
		bool IsPlayerExistAt(const Point& p) const { return this->inBounds(p) && (*this)[p].distance == 0; }
	};

	struct PlayerVisionState
	{
		bool mistRemoval{};
	};
}
