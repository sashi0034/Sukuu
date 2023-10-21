#include "stdafx.h"
#include "AutoTiler.h"

namespace Play
{
	namespace
	{
		enum TileAngle
		{
			AngL = 1 << 0,
			AngT = 1 << 1,
			AngR = 1 << 2,
			AngB = 1 << 3,
		};
	}

	constexpr Point diagonalStart{0, 4};

	consteval std::array<Point, 16> getAutoTileFormat()
	{
		std::array<Point, 16> format{};
		format[0] = {0, 3};
		format[AngB] = {0, 0};
		format[AngB | AngT] = {0, 1};
		format[AngT] = {0, 2};
		format[AngR] = {1, 3};
		format[AngR | AngL] = {2, 3};
		format[AngL] = {3, 3};
		format[AngR | AngB] = {1, 0};
		format[AngL | AngB] = {3, 0};
		format[AngR | AngT] = {1, 2};
		format[AngL | AngT] = {3, 2};
		format[AngL | AngB | AngR] = {2, 0};
		format[AngT | AngR | AngB] = {1, 1};
		format[AngT | AngL | AngB] = {3, 1};
		format[AngL | AngT | AngR] = {2, 2};
		format[AngL | AngR | AngT | AngB] = {2, 1};
		return format;
	}

	static constexpr std::array<Point, 16> autoTileFormat = getAutoTileFormat();

	Point GetAutoTileSrcPoint(int tileSize, const AutoTileConnect& connection)
	{
		const auto& point = autoTileFormat[
			connection.connectL * AngL |
			connection.connectT * AngT |
			connection.connectR * AngR |
			connection.connectB * AngB
		];
		return point * tileSize;
	}

	Point GetAutoTileDiagonalSrc(int tileSize, AutoTileDiagonal diagonal)
	{
		const auto start = tileSize * diagonalStart;
		switch (diagonal)
		{
		case AutoTileDiagonal::LT:
			return start;
		case AutoTileDiagonal::RT:
			return start.movedBy({tileSize / 2, 0});
		case AutoTileDiagonal::LB:
			return start.movedBy({0, tileSize / 2});
		case AutoTileDiagonal::RB:
			return start.movedBy({tileSize / 2, tileSize / 2});
		default: ;
			return {};
		}
	}
}
