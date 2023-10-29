#include "stdafx.h"
#include "TutorialMap.h"

namespace
{
	constexpr std::array<StringView, 31> mapStrData = {
		U"                     ", // 00
		U"    ---------------  ", // 01
		U"    -           ---  ", // 02
		U"  -----         ---  ", // 03
		U"  -----              ", // 04
		U"  -----              ", // 05
		U"  -----              ", // 06
		U"  -----              ", // 07
		U"    -                ", // 08
		U"    -------          ", // 09
		U"          -          ", // 0a
		U"          -          ", // 0b
		U"          -          ", // 0c
		U"          -          ", // 0d
		U"          -          ", // 0e
		U"          -          ", // 0f
		U"          -          ", // 10
		U"          -          ", // 11
		U"          -          ", // 12
		U"          -          ", // 13
		U"          -          ", // 14
		U"          -          ", // 15
		U"          -          ", // 16
		U"          -          ", // 17
		U"          -          ", // 18
		U"  -----------------  ", // 19
		U" ------------------- ", // 1a
		U" ---------P--------- ", // 1b
		U" ------------------- ", // 1c
		U"  -----------------  ", // 1d
		U"                     ", // 1e
	};

	constexpr int mapStrW = mapStrData[0].size();
	constexpr int mapStrH = mapStrData.size();
	constexpr auto mapStrSize = Size(mapStrW, mapStrH);

	using namespace Tutorial;

	TutorialMapData getData()
	{
		TutorialMapData data{};
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

namespace Tutorial
{
	TutorialMapData GetTutorialMap()
	{
		return getData();
	}
}
