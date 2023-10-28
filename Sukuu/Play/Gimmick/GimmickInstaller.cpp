#include "stdafx.h"
#include "GimmickInstaller.h"

namespace Play
{
	void installGimmickRandomly(GimmickGrid& gimmick, MapGrid& map, GimmickKind kind)
	{
		while (true)
		{
			const auto p = map.Rooms().RandomRoomPoint(true);
			if (gimmick[p] != GimmickKind::None) continue;
			gimmick[p] = kind;
			break;
		}
	}

	void InstallGimmicks(GimmickGrid& gimmick, MapGrid& map)
	{
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
	}
}
