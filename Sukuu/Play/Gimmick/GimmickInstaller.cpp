#include "stdafx.h"
#include "GimmickInstaller.h"

#include "Util/Dir4.h"

namespace
{
	using namespace Play;

	void installGimmickRandomly(GimmickGrid& gimmick, const MapGrid& map, GimmickKind kind)
	{
		while (true)
		{
			const auto p = map.Rooms().RandomRoomPoint(true);
			if (gimmick[p] != GimmickKind::None) continue;
			gimmick[p] = kind;
			break;
		}
	}

	void installArrowsInMaze(GimmickGrid& gimmick, const MapGrid& map)
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
}

namespace Play
{
	void InstallGimmicks(GimmickGrid& gimmick, const MapGrid& map)
	{
		if (map.Category() == MapCategory::Maze)
		{
			installArrowsInMaze(gimmick, map);
		}

		gimmick[map.Rooms().RandomRoomPoint(true)] = GimmickKind::Stairs;

		for (auto i : step(2))
		{
			installGimmickRandomly(gimmick, map, GimmickKind::Item_Helmet);
		}
		for (auto i : step(2))
		{
			installGimmickRandomly(gimmick, map, GimmickKind::Item_Pin);
		}
		for (auto i : step(2))
		{
			installGimmickRandomly(gimmick, map, GimmickKind::Item_Wing);
		}
		for (auto i : step(2))
		{
			installGimmickRandomly(gimmick, map, GimmickKind::Item_Mine);
		}
		for (auto i : step(2))
		{
			installGimmickRandomly(gimmick, map, GimmickKind::Item_LightBulb);
		}
		for (auto i : step(2))
		{
			installGimmickRandomly(gimmick, map, GimmickKind::Item_Magnet);
		}
		for (auto i : step(2))
		{
			installGimmickRandomly(gimmick, map, GimmickKind::Item_Bookmark);
		}
		for (auto i : step(2))
		{
			installGimmickRandomly(gimmick, map, GimmickKind::Item_Explorer);
		}
		for (auto i : step(2))
		{
			installGimmickRandomly(gimmick, map, GimmickKind::Item_Grave);
		}
		for (auto i : step(2))
		{
			installGimmickRandomly(gimmick, map, GimmickKind::Item_Sun);
		}
		for (auto i : step(2))
		{
			installGimmickRandomly(gimmick, map, GimmickKind::Item_Tube);
		}
		for (auto i : step(2))
		{
			installGimmickRandomly(gimmick, map, GimmickKind::Item_Solt);
		}
		for (auto i : step(2))
		{
			installGimmickRandomly(gimmick, map, GimmickKind::SemiItem_Hourglass);
		}
		for (auto i : step(2))
		{
			installGimmickRandomly(gimmick, map, GimmickKind::SemiItem_Vessel);
		}
	}
}
