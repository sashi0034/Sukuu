#include "stdafx.h"
#include "LoungeMap.h"

#include "Play/Forward.h"

namespace
{
	// 45x45
	constexpr std::array<StringView, 45> mapStrData = {
		U"                                             ",
		U"                                             ",
		U"                                             ",
		U"                                             ",
		U"                                             ",
		U"                                             ",
		U"                                             ",
		U"                                             ",
		U"                                             ",
		U"                                 *******     ",
		U"                                 *******     ",
		U"                                 *******     ",
		U"                                 ***M***     ",
		U"                                 *******     ",
		U"                                 *******     ",
		U"     *******       *******-------*******     ",
		U"     *******       *******                   ",
		U"     *******       *******                   ",
		U"     ***M***-------***M***                   ",
		U"     *******       *******                   ",
		U"     *******       *******                   ",
		U"     *******       *******-------*******     ",
		U"                      |          *******     ",
		U"                      |          *******     ",
		U"                      |          ***M***     ",
		U"                      |          *******     ",
		U"                      |          *******     ",
		U"                      |          *******     ",
		U"                   *******                   ",
		U"                   *******                   ",
		U"                   *******                   ",
		U"                   ***P***                   ",
		U"                   *******                   ",
		U"                   *******                   ",
		U"                   *******                   ",
		U"                                             ",
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

	constexpr int mapStrW = mapStrData[0].size();
	constexpr int mapStrH = mapStrData.size();
	constexpr auto mapStrSize = Size(mapStrW, mapStrH);

	using namespace Lounge;

	void applyManjiWallAt(Play::MapGrid& grid, const Point& p)
	{
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
		LoungeMapData data{};

		Play::MapGrid mapGrid{mapStrSize};
		for (const auto p : step(mapStrSize))
			mapGrid.At(p).kind = Play::TerrainKind::Wall;

		Array<Point> manjiList{};

		for (const auto p : step(mapStrSize))
		{
			mapGrid.At(p).kind = Play::TerrainKind::Floor;

			switch (mapStrData[p.y][p.x])
			{
			case U'P':
				data.initialPlayerPoint = p;
				manjiList.push_back(p);
				break;
			case U'M':
				data.manjiRegionPositions.push_back(p);
				manjiList.push_back(p);
				break;
			case ' ':
				mapGrid.At(p).kind = Play::TerrainKind::Wall;
				break;
			default:
				break;
			}
		}

		for (const auto& p : manjiList)
		{
			applyManjiWallAt(mapGrid, p);
			data.manjiRegionPositions.push_back(p * Play::CellPx_24);
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
