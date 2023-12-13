#pragma once
#include "Play/Enemy/EnemyDefinition.h"
#include "Play/Map/MapGrid.h"

namespace Play
{
	MapGrid GenerateFloorMap(int floorIndex);

	bool IsFloorExistVessel(int floorIndex);

	void GenerateEnemiesAndGimmicks(
		int floor, const MapGrid& map, ActorContainer& scene, EnemyContainer& enemyContainer, GimmickGrid& gimmick);

	std::function<ScopedCustomShader2D(double t)> GetFloorBgShader(int floor);

	bool IsFloorSnowfall(int floorIndex);

	BgmInfo GetFloorBgm(int floor);
}
