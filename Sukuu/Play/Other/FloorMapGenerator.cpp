#include "stdafx.h"
#include "FloorMapGenerator.h"

#include "Play/Enemy/EnSlimeCat.h"
#include "Play/Enemy/EnCatfish.h"
#include "Play/Enemy/EnCrab.h"
#include "Play/Enemy/EnKnight.h"
#include "Play/Enemy/EnLion.h"
#include "Play/Gimmick/GimmickInstaller.h"
#include "Play/Map/DungeonGenerator.h"
#include "Play/Map/MazeGenerator.h"

namespace
{
	using namespace Play;
}

namespace Play
{
	MapGrid GenerateFloorMap(int floorIndex)
	{
		// return GenerateFreshMaze(MazeGenProps{
		// 	.size = {65, 65},
		// });
		return GenerateFreshDungeon(DungGenProps{
			.size = {81, 81},
			.areaDivision = 8,
		});
	}
}

namespace
{
	void installEnemies(ActorView enemyParent, EnemyContainer& enemyContainer)
	{
		for (int i = 0; i < 10; ++i)
		{
			auto enemy = enemyContainer.Birth(enemyParent.AsParent(), EnSlimeCat());
			enemy.Init();
			if (i > 2) enemy.BecomePrime();
		}
		for (int i = 0; i < 10; ++i)
		{
			auto enemy = enemyContainer.Birth(enemyParent.AsParent(), EnKnight());
			enemy.Init();
			if (i > 2) enemy.BecomePrime();
		}
		for (int i = 0; i < 2; ++i)
		{
			auto enemy = enemyContainer.Birth(enemyParent.AsParent(), EnCatfish());
			enemy.Init();
		}
		for (int i = 0; i < 2; ++i)
		{
			auto enemy = enemyContainer.Birth(enemyParent.AsParent(), EnCrab());
			enemy.Init();
		}
		for (int i = 0; i < 5; ++i)
		{
			auto enemy = enemyContainer.Birth(enemyParent.AsParent(), EnLion());
			enemy.Init();
		}
	}

	void installGimmicks(const MapGrid& map, GimmickGrid& gimmick)
	{
		for (auto i : step(2))
		{
			InstallGimmickRandomly(gimmick, map, GimmickKind::Item_Helmet);
		}
		for (auto i : step(2))
		{
			InstallGimmickRandomly(gimmick, map, GimmickKind::Item_Pin);
		}
		for (auto i : step(2))
		{
			InstallGimmickRandomly(gimmick, map, GimmickKind::Item_Wing);
		}
		for (auto i : step(2))
		{
			InstallGimmickRandomly(gimmick, map, GimmickKind::Item_Mine);
		}
		for (auto i : step(2))
		{
			InstallGimmickRandomly(gimmick, map, GimmickKind::Item_LightBulb);
		}
		for (auto i : step(2))
		{
			InstallGimmickRandomly(gimmick, map, GimmickKind::Item_Magnet);
		}
		for (auto i : step(2))
		{
			InstallGimmickRandomly(gimmick, map, GimmickKind::Item_Bookmark);
		}
		for (auto i : step(2))
		{
			InstallGimmickRandomly(gimmick, map, GimmickKind::Item_Explorer);
		}
		for (auto i : step(2))
		{
			InstallGimmickRandomly(gimmick, map, GimmickKind::Item_Grave);
		}
		for (auto i : step(2))
		{
			InstallGimmickRandomly(gimmick, map, GimmickKind::Item_Sun);
		}
		for (auto i : step(2))
		{
			InstallGimmickRandomly(gimmick, map, GimmickKind::Item_Tube);
		}
		for (auto i : step(2))
		{
			InstallGimmickRandomly(gimmick, map, GimmickKind::Item_Solt);
		}
		for (auto i : step(2))
		{
			InstallGimmickRandomly(gimmick, map, GimmickKind::SemiItem_Hourglass);
		}
		for (auto i : step(2))
		{
			InstallGimmickRandomly(gimmick, map, GimmickKind::SemiItem_Vessel);
		}
	}
}

namespace Play
{
	void GenerateEnemiesAndGimmicks(
		int floor, const MapGrid& map, ActorView enemyParent, EnemyContainer& enemyContainer, GimmickGrid& gimmick)
	{
		if (map.Category() == MapCategory::Maze)
		{
			InstallArrowsInMaze(gimmick, map);
		}

		gimmick[map.Rooms().RandomRoomPoint(true)] = GimmickKind::Stairs;

		installGimmicks(map, gimmick);

		installEnemies(enemyParent, enemyContainer);
	}
}
