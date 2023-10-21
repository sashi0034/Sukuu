#pragma once

namespace Play
{
	struct AutoTileConnect
	{
		bool connectL;
		bool connectT;
		bool connectR;
		bool connectB;
	};

	enum class AutoTileDiagonal
	{
		LT = 0,
		RT = 1,
		LB = 2,
		RB = 3,
	};

	Point GetAutoTileSrcPoint(int tileSize, const AutoTileConnect& connection);

	Point GetAutoTileDiagonalSrc(int tileSize, AutoTileDiagonal diagonal);
}
