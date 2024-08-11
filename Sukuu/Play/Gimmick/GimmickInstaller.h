#pragma once
#include "GimmickGrid.h"
#include "Play/Map/MapGrid.h"

namespace Play
{
	void InstallArrowsInMaze(GimmickGrid& gimmick, const MapGrid& map);
	void InstallArrowsInDungeon(GimmickGrid& gimmick, const MapGrid& map);
	void InstallTreesInDungeon(GimmickGrid& gimmick, const MapGrid& map);
	bool InstallGimmickRandomly(GimmickGrid& gimmick, const MapGrid& map, GimmickKind kind);
}
