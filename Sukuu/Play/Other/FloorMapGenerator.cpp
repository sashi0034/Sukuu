#include "stdafx.h"
#include "FloorMapGenerator.h"

#include <Siv3D/ScriptFunction.hpp>

#include "AssetKeys.h"
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
		// 迷路: 7、13, 23, 31, 41

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
	bool canExistItem(ConsumableItem item, int floor)
	{
		switch (item)
		{
		case ConsumableItem::Wing:
			return floor >= 5;
		case ConsumableItem::Helmet:
			return true;
		case ConsumableItem::Pin:
			return true;
		case ConsumableItem::Mine:
			return floor >= 5;
		case ConsumableItem::LightBulb:
			return floor >= 10;
		case ConsumableItem::Magnet:
			return true;
		case ConsumableItem::Bookmark:
			return floor >= 10;
		case ConsumableItem::Explorer:
			return floor >= 15;
		case ConsumableItem::Grave:
			return floor >= 5;
		case ConsumableItem::Sun:
			return floor >= 10;
		case ConsumableItem::Tube:
			return floor >= 15;
		case ConsumableItem::Solt:
			return floor >= 5;
		default: ;
			return false;
		}
	}

	int getRoomItemCount(int floorIndex)
	{
		if (floorIndex == 1) return 3;
		if (floorIndex <= 3) return Random(3, 5);
		if (floorIndex <= 6) return Random(3, 7);
		if (floorIndex == 7) return 18;
		if (floorIndex <= 12) return Random(5, 11);
		if (floorIndex == 13) return 24;
		if (floorIndex <= 22) return Random(3, 13);
		if (floorIndex == 23) return 24;
		if (floorIndex <= 30) return Random(5, 9);
		if (floorIndex == 31) return 24;
		if (floorIndex <= 40) return Random(3, 11);
		if (floorIndex == 41) return 18;
		return Random(3, 7);
	}

	GimmickKind getRandomItemGimmick()
	{
		while (true)
		{
			switch (static_cast<ConsumableItem>(Random(1, static_cast<int>(ConsumableItem::Max) - 1)))
			{
			case ConsumableItem::Wing:
				if (RandomBool(0.7)) return GimmickKind::Item_Wing;
			case ConsumableItem::Helmet:
				return GimmickKind::Item_Helmet;
			case ConsumableItem::Pin:
				return GimmickKind::Item_Pin;
			case ConsumableItem::Mine:
				return GimmickKind::Item_Mine;
			case ConsumableItem::LightBulb:
				return GimmickKind::Item_LightBulb;
			case ConsumableItem::Magnet:
				return GimmickKind::Item_Magnet;
			case ConsumableItem::Bookmark:
				if (RandomBool(0.7)) return GimmickKind::Item_Bookmark;
			case ConsumableItem::Explorer:
				if (RandomBool(0.7)) return GimmickKind::Item_Explorer;
			case ConsumableItem::Grave:
				return GimmickKind::Item_Grave;
			case ConsumableItem::Sun:
				return GimmickKind::Item_Sun;
			case ConsumableItem::Tube:
				if (RandomBool(0.5)) return GimmickKind::Item_Tube;
			case ConsumableItem::Solt:
				if (RandomBool(0.7)) return GimmickKind::Item_Solt;
			default: ;
				break;
			}
		}
	}

	int getRoomEnemyCount(int floorIndex)
	{
		if (floorIndex == 1) return 2;
		if (floorIndex <= 3) return Random(4, 8);
		if (floorIndex <= 6) return Random(4, 12);
		if (floorIndex == 7) return 18;
		if (floorIndex <= 12) return Random(8, 16);
		if (floorIndex == 13) return 20;
		if (floorIndex <= 22) return Random(10, 16);
		if (floorIndex == 23) return 24;
		if (floorIndex <= 30) return Random(12, 18);
		if (floorIndex == 31) return 28;
		if (floorIndex <= 40) return Random(12, 20);
		if (floorIndex == 41) return 32;
		return Random(12, 24);
	}

	bool tryRandomBirthEnemy(ActorView enemyParent, EnemyContainer& enemyContainer, int floorIndex)
	{
		switch (static_cast<EnemyKind>(Random(0, static_cast<int>(EnemyKind::Max) - 1)))
		{
		case EnemyKind::SlimeCat: {
			auto enemy = enemyContainer.Birth(enemyParent.AsParent(), EnSlimeCat());
			enemy.Init();
			return true;
		}
		case EnemyKind::Knight: {
			if (floorIndex <= 1) return false;
			if (RandomBool(0.4)) return false;
			auto enemy = enemyContainer.Birth(enemyParent.AsParent(), EnKnight());
			enemy.Init();
			return true;
		}
		case EnemyKind::Catfish: {
			if (floorIndex <= 5) return false;
			if (RandomBool(0.8)) return false;
			auto enemy = enemyContainer.Birth(enemyParent.AsParent(), EnCatfish());
			enemy.Init();
			return true;
		}
		case EnemyKind::Crab: {
			if (floorIndex <= 15) return false;
			if (RandomBool(0.6)) return false;
			auto enemy = enemyContainer.Birth(enemyParent.AsParent(), EnCrab());
			enemy.Init();
			return true;
		}
		case EnemyKind::Lion: {
			if (floorIndex <= 25) return false;
			if (RandomBool(0.4)) return false;
			auto enemy = enemyContainer.Birth(enemyParent.AsParent(), EnLion());
			enemy.Init();
			return true;
		}
		case EnemyKind::SlimeCat_prime: {
			if (floorIndex <= 35) return false;
			auto enemy = enemyContainer.Birth(enemyParent.AsParent(), EnSlimeCat());
			enemy.Init();
			enemy.BecomePrime();
			return true;
		}
		case EnemyKind::Knight_prime: {
			if (floorIndex <= 40) return false;
			if (RandomBool(0.4)) return false;
			auto enemy = enemyContainer.Birth(enemyParent.AsParent(), EnKnight());
			enemy.Init();
			enemy.BecomePrime();
			return true;
		}
		default: ;
			return false;
		};
	}


	void installEnemies(ActorView enemyParent, EnemyContainer& enemyContainer, int floorIndex)
	{
		const int enemyCount = getRoomEnemyCount(floorIndex);

		for (const auto i : step(enemyCount))
		{
			while (true)
			{
				if (tryRandomBirthEnemy(enemyParent, enemyContainer, floorIndex)) break;
			}
		}
	}

	bool isExistTreeFloor(const MapGrid& map, int floorIndex)
	{
		if (map.Category() != MapCategory::Dungeon) return false;
		return
			InRange(floorIndex, 16, 20) ||
			InRange(floorIndex, 26, 30);
	}

	void installTrees(const MapGrid& map, GimmickGrid& gimmick, int floorIndex)
	{
		if (isExistTreeFloor(map, floorIndex))
		{
			InstallTreesInDungeon(gimmick, map);
		}
	}

	void installGimmicks(const MapGrid& map, GimmickGrid& gimmick, int floorIndex)
	{
		if (IsExistVesselFloor(floorIndex))
		{
			InstallGimmickRandomly(gimmick, map, GimmickKind::SemiItem_Vessel);
		}

		const int numPops = getRoomItemCount(floorIndex);
		for (const auto i : step(numPops))
		{
			if (RandomBool(0.4))
			{
				InstallGimmickRandomly(gimmick, map, GimmickKind::SemiItem_Hourglass);
				continue;
			}
			GimmickKind item{};
			while (true)
			{
				item = getRandomItemGimmick();
				if (canExistItem(GimmickToItem(item), floorIndex)) break;
			}
			InstallGimmickRandomly(gimmick, map, item);
		}
	}

	struct RgbToBgrCb
	{
		float rate;
	};

	struct GrayscaleCb
	{
		float rate;
	};
}

namespace Play
{
	bool IsExistVesselFloor(int floorIndex)
	{
		return
			floorIndex == 7 ||
			floorIndex == 13 ||
			floorIndex == 17 ||
			floorIndex == 29 ||
			floorIndex == 31 ||
			floorIndex == 37;
	}

	void GenerateEnemiesAndGimmicks(
		int floor, const MapGrid& map, ActorView scene, EnemyContainer& enemyContainer, GimmickGrid& gimmick)
	{
		if (map.Category() == MapCategory::Maze)
		{
			InstallArrowsInMaze(gimmick, map);
		}

		gimmick[map.Rooms().RandomRoomPoint(true)] = GimmickKind::Stairs;

		installGimmicks(map, gimmick, floor);

		installEnemies(scene, enemyContainer, floor);

		installTrees(map, gimmick, floor);
	}

	std::function<ScopedCustomShader2D(double t)> GetFloorBgShader(int floor)
	{
		if (InRange(floor, 24, 28))
		{
			return [&](double t)
			{
				ConstantBuffer<RgbToBgrCb> cb{};
				cb->rate = 0.9 + 0.1 * static_cast<float>(Periodic::Sine1_1(20.0s, t));
				Graphics2D::SetPSConstantBuffer(1, cb);
				return ScopedCustomShader2D(PixelShaderAsset(AssetKeys::PsRgbToBgr));
			};
		}
		if (floor == 41)
		{
			return [&](double t)
			{
				ConstantBuffer<RgbToBgrCb> cb{};
				cb->rate = 0.5 + 0.1 * static_cast<float>(Periodic::Sine1_1(20.0s, t));
				Graphics2D::SetPSConstantBuffer(1, cb);
				return ScopedCustomShader2D(PixelShaderAsset(AssetKeys::PsRgbToBgr));
			};
		}
		if (InRange(floor, 44, 48))
		{
			return [&](double t)
			{
				ConstantBuffer<GrayscaleCb> cb{};
				cb->rate = 0.7 + 0.3 * static_cast<float>(Periodic::Sine0_1(10.0s, t));
				Graphics2D::SetPSConstantBuffer(1, cb);
				return ScopedCustomShader2D(PixelShaderAsset(AssetKeys::PsGrayscale));
			};
		}
		return [](double t) { return ScopedCustomShader2D{}; };
	}
}
