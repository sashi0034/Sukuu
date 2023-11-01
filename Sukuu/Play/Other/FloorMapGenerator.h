#pragma once
#include "Play/Enemy/EnemyDefinition.h"
#include "Play/Map/MapGrid.h"

namespace Play
{
	MapGrid GenerateFloorMap(int floorIndex);

	void GenerateEnemiesAndGimmicks(
		int floor, const MapGrid& map, ActorView enemyParent, EnemyContainer& enemyContainer, GimmickGrid& gimmick);
}
