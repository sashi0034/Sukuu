#include "stdafx.h"
#include "GimmickInstaller.h"

#include "Util/Dir4.h"

namespace
{
	using namespace Play;
}

namespace Play
{
	void InstallArrowsInMaze(GimmickGrid& gimmick, const MapGrid& map)
	{
		for (const auto p : step({1, 1}, gimmick.size().moveBy(-2, -2)))
		{
			if (map.Data()[p].kind == TerrainKind::Wall) continue;;

			int wallCount = 0;
			Dir4Type movableDir{Dir4::Invalid};
			for (int dir = 0; dir < 4; ++dir)
			{
				if (map.Data()[p.movedBy(Dir4Type(dir).ToXY().asPoint())].kind == TerrainKind::Wall) wallCount++;
				else movableDir = dir;
			}
			if (wallCount != 3) continue;

			switch (movableDir.Reversed())
			{
			case Dir4::Right:
				gimmick[p] = GimmickKind::Arrow_right;
				break;
			case Dir4::Up:
				gimmick[p] = GimmickKind::Arrow_up;
				break;
			case Dir4::Left:
				gimmick[p] = GimmickKind::Arrow_left;
				break;
			case Dir4::Down:
				gimmick[p] = GimmickKind::Arrow_down;
				break;
			default: break;
			}
		}
	}

	void InstallTreesInDungeon(GimmickGrid& gimmick, const MapGrid& map)
	{
		for (int x = 1; x < gimmick.size().x; x += 2)
		{
			for (int y = 1; y < gimmick.size().y; y += 2)
			{
				if (map.At({x, y}).kind != TerrainKind::Wall) continue;

				const int c = (x % 4) + (y % 4);
				if (c == 2 || c == 6)
				{
					gimmick[y][x] = GimmickKind::Tree_large;
				}
				else
				{
					gimmick[y][x] = GimmickKind::Tree_small;
				}
			}
		}
	}

	bool InstallGimmickRandomly(GimmickGrid& gimmick, const MapGrid& map, GimmickKind kind)
	{
		int failedCount{};
		while (true)
		{
			const auto p = map.Rooms().RandomRoomPoint(true);
			if (gimmick[p] == GimmickKind::None)
			{
				gimmick[p] = kind;
				return true;
			}
			failedCount++;
			if (failedCount >= 1000) return false;
		}
	}
}
