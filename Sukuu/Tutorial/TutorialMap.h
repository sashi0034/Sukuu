#pragma once
#include "Play/Map/MapGrid.h"

namespace Tutorial
{
	struct TutorialMapData
	{
		Play::MapGrid map;
		Point initialPlayerPoint;
		Point firstBlockPoint;
		Point catSpawnPoint_North;
		Point catSpawnPoint_South;
		Point sukuuEventPoint;
		Point itemSpawnPoint;
		Point knightBlockPoint;
		Point knightSpawnPoint;
		Point stairsPoint;
		Point finalEventPoint;
	};

	TutorialMapData GetTutorialMap();
}
