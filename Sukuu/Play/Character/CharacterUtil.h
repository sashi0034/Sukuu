#pragma once
#include "Play/Forward.h"
#include "Play/Map/MapGrid.h"
#include "Util/Dir4.h"

namespace Play
{
	class CharacterPos : public Vec2
	{
	public:
		using Vec2::Vec2;
		CharacterPos(Vec2 v): Vector2D<double>(v.x, v.y) { return; }

		Point MapPoint() const
		{
			return this->asPoint() / CellPx_24;
			// return (this->asPoint() + Point{CellPx_24 / 2, CellPx_24 / 2}) / CellPx_24;
		}
	};

	bool CanMoveTo(const MapGrid& map, const CharacterPos& current, Dir4Type dir);
}
