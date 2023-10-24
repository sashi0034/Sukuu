#include "stdafx.h"
#include "MapGrid.h"

namespace Play
{
	Point MapRooms::RandomRoomPoint(bool isOdd) const
	{
		return RandomRoomPoint(Random(0, static_cast<int>((*this).size() - 1)), isOdd);
	}

	Point MapRooms::RandomRoomPoint(int index, bool isOdd) const
	{
		auto&& size = (*this)[index].size;
		const int odd = isOdd ? 1 : 0;
		return (*this)[index].tl()
			+ Point{Random(0, size.x - 1 - odd), Random(0, size.y - 1 - odd)} / 2 * 2
			+ Point{odd, odd};
	}
}
