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
			U"                                             ",
			U"                                             ",
			U"                                T       T    ",
			U"                               2    T    3   ",
			U"                                 *******     ",
			U"                                 *******     ",
			U"                                 *******     ",
			U"                      T     T    ***M***     ",
			U"                         T   R   *******     ",
			U"    R                      R   R *******     ",
			U"     *******  0    *******-------*******     ",
			U"     *******    1  *******                   ",
			U" 0   *******       *******                   ",
			U"     ***M***-------***M***  3        1       ",
			U"  0  ******* T   T ******* 2       0   1     ",
			U"     *******   T   *******                   ",
			U"     *******       *******-------*******     ",
			U"     U U U U          |          *******     ",
			U"    U U U U U         |          *******     ",
			U"                 T    |    T     ***M***     ",
			U"                  U   |   U      *******     ",
			U"                T     |     T    *******     ",
			U"                      S          *******     ",
			U"                 R ******* R                 ",
			U"                   *******          T     T  ",
			U"               U   *******             T     ",
			U"             U     ***P***                   ",
			U"               U   *******   1               ",
			U"                   ******* 0                 ",
			U"                0  *******                   ",
			U"                 1                           ",
			U"                                             ",
			U"                                             ",
			U"                                             ",
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
				data.bigTreePositions.push_back(p * Play::CellPx_24);
				break;
			case 'U':
				data.smallTreePositions.push_back(p * Play::CellPx_24);
				break;
			case'0':
				data.mixedNaturePositions.push_back({Point{2, 0} * 16, p * Play::CellPx_24});
				break;
			case'1':
				data.mixedNaturePositions.push_back({Point{3, 0} * 16, p * Play::CellPx_24});
				break;
			case'2':
				data.mixedNaturePositions.push_back({Point{2, 1} * 16, p * Play::CellPx_24});
				break;
			case'3':
				data.mixedNaturePositions.push_back({Point{3, 1} * 16, p * Play::CellPx_24});
				break;
			// case ' ':
			// 	mapGrid.At(p).kind = Play::TerrainKind::Wall;
			// 	break;
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
