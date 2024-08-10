#pragma once
#include "Play/Map/MapGrid.h"

namespace Lounge
{
	enum class LoungeBridgeKind
	{
		// Horizontal
		Hl,
		Hc,
		Hr,
		// Vertical
		Vt,
		Vm,
		Vb,
	};

	struct LoungeBridgeTuple
	{
		LoungeBridgeKind kind;
		Vec2 position;
		int hash;
	};

	struct LoungeMapData
	{
		Play::MapGrid map{};
		Point initialPlayerPoint{};

		Array<Vec2> manjiRegionPositions{};
		Array<LoungeBridgeTuple> bridgePositions{};
		Array<Vec2> toriiPositions{};
		Array<Vec2> tourouPositions{};

		Array<Vec2> bigTreePositions{};
		Array<Vec2> smallTreePositions{};
	};

	LoungeMapData GetLoungeMap();
}
