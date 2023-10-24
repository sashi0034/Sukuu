#pragma once

#include "Play/Gimmick/ConsumableItem.h"
#include "Play/Forward.h"

namespace Play
{
	constexpr int PlayerDistanceInfinity = INT32_MAX / 2;

	struct PlayerPosRelation
	{
		int distance = PlayerDistanceInfinity;
		bool directStraight = false;
	};

	struct PlayerPersonalData
	{
		std::array<ConsumableItem, MaxItemPossession> items;
	};

	using PlayerDistField = Grid<PlayerPosRelation>;
}
