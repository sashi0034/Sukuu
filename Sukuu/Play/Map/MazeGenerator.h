#pragma once
#include "MapGrid.h"

namespace Play
{
	struct MazeGenProps
	{
		Size size;
		int numRooms = 10;
		int roomSize = 5;
	};

	MapGrid GenerateFreshMaze(const MazeGenProps& props);
}
