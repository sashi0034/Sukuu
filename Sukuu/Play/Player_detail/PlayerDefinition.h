#pragma once

namespace Play
{
	constexpr int PlayerDistanceInfinity = INT32_MAX / 2;

	struct PlayerPosRelation
	{
		int distance = PlayerDistanceInfinity;
		bool directStraight = false;
	};

	using PlayerDistField = Grid<PlayerPosRelation>;
}
