#include "stdafx.h"
#include "LoungeMap.h"

namespace
{
	constexpr std::array<StringView, 65> mapStrData = {
		U"                                                                 ",
		U"                                                                 ",
		U"                                                                 ",
		U"                                                                 ",
		U"                                                                 ",
		U"                                                                 ",
		U"                                                                 ",
		U"                                                                 ",
		U"                              -----                   -----      ",
		U"                              -----                   --S--      ",
		U"                            ---------               -------      ",
		U"                            ---------               -------      ",
		U"                C----------i----I-------3K--------------g--      ",
		U"                -           ---------               -------      ",
		U"                -           ---------               -------      ",
		U"                -             -----                   --g--      ",
		U"                -             -----                   -----      ",
		U"                -                                                ",
		U"                -                                                ",
		U"                -                                                ",
		U"                -                                                ",
		U"                -                                                ",
		U"                -                                                ",
		U"                -                                                ",
		U"                1                                                ",
		U"                -                                                ",
		U"                -                                                ",
		U"                -                                                ",
		U"                -                                                ",
		U"                -                                                ",
		U"                -                                                ",
		U"                -                                                ",
		U"                -                                                ",
		U"                -                                                ",
		U"                -                                                ",
		U"                -                                                ",
		U"                c                                                ",
		U"                -                                                ",
		U"                -                                                ",
		U"                -                                                ",
		U"                -----------------                                ",
		U"                                -                                ",
		U"                                -                                ",
		U"                                -                                ",
		U"                                -                                ",
		U"                                -                                ",
		U"                                -                                ",
		U"                                -                                ",
		U"                                -                                ",
		U"                                -                                ",
		U"                                -                                ",
		U"                                0                                ",
		U"                          -------------                          ",
		U"                          -------------                          ",
		U"                        -----------------                        ",
		U"                        -----------------                        ",
		U"                        --------P--------                        ",
		U"                        -----------------                        ",
		U"                        -----------------                        ",
		U"                          -------------                          ",
		U"                          -------------                          ",
		U"                                                                 ",
		U"                                                                 ",
		U"                                                                 ",
		U"                                                                 ",
	};

	constexpr int mapStrW = mapStrData[0].size();
	constexpr int mapStrH = mapStrData.size();
	constexpr auto mapStrSize = Size(mapStrW, mapStrH);

	using namespace Lounge;

	LoungeMapData getData()
	{
		LoungeMapData data{};

		Play::MapGrid mapGrid{mapStrSize};
		for (const auto p : step(mapStrSize))
			mapGrid.At(p).kind = Play::TerrainKind::Wall;

		for (const auto p : step(mapStrSize))
		{
			mapGrid.At(p).kind = Play::TerrainKind::Floor;

			switch (mapStrData[p.y][p.x])
			{
			case U'P':
				data.initialPlayerPoint = p;
				break;
			case ' ':
				mapGrid.At(p).kind = Play::TerrainKind::Wall;
				break;
			default:
				break;
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
