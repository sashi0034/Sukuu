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
		if (floorIndex == 1)
		{
			return GenerateFreshDungeon(DungGenProps{
				.size = {41, 41},
				.areaDivision = 4,
				. minAreaSize = 20,
				.minAreaWidthHeight = 12,
				.minRoomSize = 10,
				.minRoomWidthHeight = 6,
			});
		}
		if (floorIndex <= 3)
		{
			return GenerateFreshDungeon(DungGenProps{
				.size = {51, 51},
				.areaDivision = 5,
				.minAreaSize = 20,
				.minAreaWidthHeight = 12,
				.minRoomSize = 10,
				.minRoomWidthHeight = 6,
			});
		}
		if (floorIndex <= 6)
		{
			return GenerateFreshDungeon(DungGenProps{
				.size = {61, 61},
				.areaDivision = 6,
				.minAreaSize = 28,
				.minAreaWidthHeight = 12,
				.minRoomSize = 14,
				.minRoomWidthHeight = 6,
			});
		}
		if (floorIndex == 7)
		{
			return GenerateFreshMaze(MazeGenProps{
				.size = {55, 55},
				.numRooms = 8,
			});
		}
		if (floorIndex <= 12)
		{
			return GenerateFreshDungeon(DungGenProps{
				.size = {71, 71},
				.areaDivision = 7,
				.minAreaSize = 28,
				.minAreaWidthHeight = 16,
				.minRoomSize = 14,
				.minRoomWidthHeight = 8,
			});
		}
		if (floorIndex == 13)
		{
			return GenerateFreshMaze(MazeGenProps{
				.size = {65, 65},
				.numRooms = 8,
			});
		}
		if (floorIndex == 14)
		{
			return GenerateFreshDungeon(DungGenProps{
				.size = {65, 65},
				.areaDivision = 4,
				.minAreaSize = 24,
				.minAreaWidthHeight = 16,
				.minRoomSize = 12,
				.minRoomWidthHeight = 8,
			});
		}
		if (floorIndex <= 22)
		{
			return GenerateFreshDungeon(DungGenProps{
				.size = {81, 81},
				.areaDivision = 8,
			});
		}
		if (floorIndex == 23)
		{
			return GenerateFreshMaze(MazeGenProps{
				.size = {75, 75},
				.numRooms = 10,
			});
		}
		if (floorIndex <= 30)
		{
			return GenerateFreshDungeon(DungGenProps{
				.size = {91, 91},
				.areaDivision = 9 + Random(-2, 2),
			});
		}
		if (floorIndex == 31)
		{
			return GenerateFreshMaze(MazeGenProps{
				.size = {85, 85},
				.numRooms = 12,
			});
		}
		if (floorIndex <= 36)
		{
			return GenerateFreshDungeon(DungGenProps{
				.size = {101, 101},
				.areaDivision = 10 + Random(-2, 2),
			});
		}
		if (floorIndex <= 40)
		{
			return GenerateFreshDungeon(DungGenProps{
				.size = {81, 81},
				.areaDivision = 8 + Random(-2, 2),
				.minAreaSize = 28,
				.minAreaWidthHeight = 16,
				.minRoomSize = 14,
				.minRoomWidthHeight = 8,
			});
		}
		if (floorIndex == 41)
		{
			return GenerateFreshMaze(MazeGenProps{
				.size = {75, 75},
				.numRooms = 12,
			});
		}
		if (floorIndex <= 46)
		{
			return GenerateFreshDungeon(DungGenProps{
				.size = {91, 91},
				.areaDivision = 6,
				.minAreaSize = 28,
				.minAreaWidthHeight = 16,
				.minRoomSize = 14,
				.minRoomWidthHeight = 8,
				.areaRoomPadding = 6,
			});
		}
		if (floorIndex == 47)
		{
			return GenerateFreshDungeon(DungGenProps{
				.size = {101, 101},
				.areaDivision = 6,
			});
		}
		return GenerateFreshDungeon(DungGenProps{
			.size = {101, 101},
			.areaDivision = 8 + Random(-1, 1),
			.areaRoomPadding = 6,
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
