#include "stdafx.h"
#include "CharacterUtil.h"

namespace Play
{
	bool CanMoveTo(const MapGrid& map, const CharacterPos& current, Dir4Type dir)
	{
		const Point next = current.MapPoint() + dir.ToXY().asPoint();
		if (map.Data().inBounds(next) == false) return false;
		return map.At(next).kind != TerrainKind::Wall;
	}
}
