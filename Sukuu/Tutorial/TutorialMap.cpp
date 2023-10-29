#include "stdafx.h"
#include "TutorialMap.h"

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
		U"                              -----                   -----      ",
		U"                            ---------               -------      ",
		U"                            ---------               -------      ",
		U"                C---------------I-------3K--------------S--      ",
		U"                -           ---------               -------      ",
		U"                -           ---------               -------      ",
		U"                -             -----                   -----      ",
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
			case U'0':
				data.firstBlockPoint = p;
				break;
			case U'c':
				data.catSpawnPoint_South = p;
				break;
			case U'C':
				data.catSpawnPoint_North = p;
				break;
			case U'1':
				data.sukuuEventPoint = p;
				break;
			case U'I':
				data.itemSpawnPoint = p;
				break;
			case U'3':
				data.knightBlockPoint = p;
				break;
			case U'K':
				data.knightSpawnPoint = p;
				break;
			case U'S':
				data.stairsPoint = p;
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
