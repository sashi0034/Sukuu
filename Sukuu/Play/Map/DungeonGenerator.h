#pragma once
#include "MapGrid.h"

namespace Play
{
	struct DungGenProps
	{
		Size size;
		int areaDivision;
		uint32 minAreaSize = 40;
		uint32 minAreaWidthHeight = 12;
		uint32 minRoomSize = 20;
		uint32 minRoomWidthHeight = 6;
		uint32 areaRoomPadding = 4;
	};

	MapGrid GenerateFreshDungeon(const DungGenProps& props);
}
