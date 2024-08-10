#include "stdafx.h"
#include "LoungeMap.h"

#include "Play/Forward.h"

namespace
{
	// 45x45
	Array<String> mapStringData()
	{
		return {
			U"                                             ",
			U"                                             ",
			U"                                             ",
			U"                                             ",
			U"                                             ",
			U"                         T   T   T   T   T   ",
			U"                       T   T   T   T   T   T ",
			U"                         T   T   T   T   T   ",
			U"                       T   T   T   T   T     ",
			U"  T                      T   T   *******     ",
			U"    T                            ******* R   ",
			U"  T       T   T                  *******   R ",
			U"        T   T   T     T     T    ***M*** R   ",
			U"   R R    T   T   T      T   R   *******     ",
			U"    R       T   T   T      R   R *******     ",
			U"     *******       *******-------*******  T  ",
			U"     *******       *******              T    ",
			U" T   ******* R   R ******* T   T          T  ",
			U"     ***M***-------***M***   T       T       ",
			U"  T  ******* T   T *******         T   T     ",
			U"     *******   T   *******                   ",
			U" T   *******       *******-------******* T   ",
			U"    R                 |          *******  T  ",
			U"      R            R  |  R       ******* T   ",
			U"    R            T    |    T     ***M***  T  ",
			U" T   T       T        |       T  ******* T   ",
			U"   T   T        T  R  |  R  T    *******     ",
			U"     T   T            S          *******     ",
			U"                 R ******* R                 ",
			U"                   *******          T     T  ",
			U"               T   *******             T     ",
			U"             T     ***P***                   ",
			U"           T   T   *******   T   T           ",
			U"             T     ******* T   T   T         ",
			U"           T       *******                   ",
			U"                                             ",
			U"              T   T   T   T   T   T          ",
			U"                T   T   T   T   T   T        ",
			U"                  T   T   T   T   T   T      ",
			U"                                             ",
			U"                                             ",
			U"                                             ",
			U"                                             ",
			U"                                             ",
			U"                                             ",
		};
	}

	using namespace Lounge;

	void applyManjiFloorAt(Play::MapGrid& grid, const Point& p)
	{
		for (int x = -3; x <= 3; ++x)
		{
			for (int y = -3; y <= 3; ++y)
			{
				grid.At(p.movedBy(x, y)).kind = Play::TerrainKind::Floor;
			}
		}

		/* 「卍」の形に壁を配置する
		 * 0000100
		 * 0000100
		 * 1100000
		 * 000M000
		 * 0000011
		 * 0010000
		 * 0010000
		 */

		grid.At(p.movedBy(-2, -1)).kind = Play::TerrainKind::Wall;
		grid.At(p.movedBy(-3, -1)).kind = Play::TerrainKind::Wall;

		grid.At(p.movedBy(1, -2)).kind = Play::TerrainKind::Wall;
		grid.At(p.movedBy(1, -3)).kind = Play::TerrainKind::Wall;

		grid.At(p.movedBy(2, 1)).kind = Play::TerrainKind::Wall;
		grid.At(p.movedBy(3, 1)).kind = Play::TerrainKind::Wall;

		grid.At(p.movedBy(-1, 2)).kind = Play::TerrainKind::Wall;
		grid.At(p.movedBy(-1, 3)).kind = Play::TerrainKind::Wall;
	}

	LoungeMapData getData()
	{
		const auto mapStrData = mapStringData();
		const int mapStrW = mapStrData[0].size();
		const int mapStrH = mapStrData.size();
		const auto mapStrSize = Size(mapStrW, mapStrH);

		LoungeMapData data{};

		Play::MapGrid mapGrid{mapStrSize};
		for (const auto p : step(mapStrSize))
			mapGrid.At(p).kind = Play::TerrainKind::Wall;

		Array<Point> manjiList{};
		Array<Point> horizontalBridgeList{};
		Array<Point> verticalBridgeList{};

		for (const auto p : step(mapStrSize))
		{
			mapGrid.At(p).kind = Play::TerrainKind::Wall;

			switch (mapStrData[p.y][p.x])
			{
			case U'P':
				data.initialPlayerPoint = p;
				manjiList.push_back(p);
				break;
			case U'M':
				manjiList.push_back(p);
				break;
			case U'S':
				data.toriiPositions.push_back(p * Play::CellPx_24);
				verticalBridgeList.push_back(p);
				break;
			case U'-':
				horizontalBridgeList.push_back(p);
				break;
			case U'|':
				verticalBridgeList.push_back(p);
				break;
			case 'R':
				data.tourouPositions.push_back(p * Play::CellPx_24);
				break;
			case 'T':
				data.treePositions.push_back(p * Play::CellPx_24);
				break;
			default:
				break;
			}
		}

		for (const auto& p : manjiList)
		{
			applyManjiFloorAt(mapGrid, p);
			data.manjiRegionPositions.push_back(p * Play::CellPx_24);
		}

		for (const auto& p : horizontalBridgeList)
		{
			mapGrid.At(p).kind = Play::TerrainKind::Floor;
			const int hash = p.x + p.y;

			if (not horizontalBridgeList.includes({p.movedBy(-1, 0)}))
			{
				data.bridgePositions.push_back({LoungeBridgeKind::Hl, p * Play::CellPx_24, hash});
			}
			else if (not horizontalBridgeList.includes({p.movedBy(1, 0)}))
			{
				data.bridgePositions.push_back({LoungeBridgeKind::Hr, p * Play::CellPx_24, hash});
			}
			else
			{
				data.bridgePositions.push_back({LoungeBridgeKind::Hc, p * Play::CellPx_24, hash});
			}
		}

		for (const auto& p : verticalBridgeList)
		{
			mapGrid.At(p).kind = Play::TerrainKind::Floor;
			const int hash = p.x + p.y;

			if (not verticalBridgeList.includes({p.movedBy(0, -1)}))
			{
				data.bridgePositions.push_back({LoungeBridgeKind::Vt, p * Play::CellPx_24, hash});
			}
			else if (not verticalBridgeList.includes({p.movedBy(0, 1)}))
			{
				data.bridgePositions.push_back({LoungeBridgeKind::Vb, p * Play::CellPx_24, hash});
			}
			else
			{
				data.bridgePositions.push_back({LoungeBridgeKind::Vm, p * Play::CellPx_24, hash});
			}
		}

		data.map = std::move(mapGrid);
		return data;
	}
}

namespace Lounge
{
	LoungeMapData GetLoungeMap()
	{
		return getData();
	}
}
