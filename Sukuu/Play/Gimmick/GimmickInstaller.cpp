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

	void InstallGimmickRandomly(GimmickGrid& gimmick, const MapGrid& map, GimmickKind kind)
	{
		while (true)
		{
			const auto p = map.Rooms().RandomRoomPoint(true);
			if (gimmick[p] != GimmickKind::None) continue;
			gimmick[p] = kind;
			break;
		}
	}
}
