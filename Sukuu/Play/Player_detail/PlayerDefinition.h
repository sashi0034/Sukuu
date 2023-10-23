#pragma once

namespace Play
{
	struct PlayerPosRelation
	{
		int distance = -1;
		bool directStraight = false;
	};

	using PlayerDistField = Grid<PlayerPosRelation>;
}
