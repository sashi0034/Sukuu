#pragma once
#include "Play/Map/MapGrid.h"

namespace Lounge
{
	struct LoungeMapData
	{
		Play::MapGrid map;
		Point initialPlayerPoint;
	};

	LoungeMapData GetLoungeMap();
}
