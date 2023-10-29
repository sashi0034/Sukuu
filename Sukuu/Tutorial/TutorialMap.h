#pragma once
#include "Play/Map/MapGrid.h"

namespace Tutorial
{
	struct TutorialMapData
	{
		Play::MapGrid map;
		Point initialPlayerPoint;
	};

	TutorialMapData GetTutorialMap();
}
