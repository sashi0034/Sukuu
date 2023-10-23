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
		return (*this)[index].tl() + Point{Random(0, size.x), Random(0, size.y)} / 2 * 2
			+ (isOdd ? Point{1, 1} : Point{0, 0});
	}
}
