#pragma once
#include "Play/Map/MapGrid.h"

namespace Tutorial
{
	struct TutorialMapData
	{
		Play::MapGrid map;
		Array<Point> hourglassPoints;
		Point initialPlayerPoint;
		Point firstBlockPoint;
		Point catSpawnPoint_North;
		Point catSpawnPoint_South;
		Point sukuuEventPoint;
		Point itemBlockPoint;
		Point itemSpawnPoint;
		Point knightBlockPoint;
		Point knightSpawnPoint;
		Point stairsPoint;
	};

	TutorialMapData GetTutorialMap();
}
