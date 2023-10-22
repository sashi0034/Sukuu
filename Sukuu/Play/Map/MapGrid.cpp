#include "stdafx.h"
#include "MapGrid.h"

namespace Play
{
	Point MapRooms::RandomRoomPoint() const
	{
		return RandomRoomPoint(Random(0, static_cast<int>((*this).size() - 1)));
	}

	Point MapRooms::RandomRoomPoint(int index) const
	{
		auto&& size = (*this)[index].size;
		return (*this)[index].tl() + Point{Random(0, size.x), Random(0, size.y)} / 2 * 2;
	}
}
