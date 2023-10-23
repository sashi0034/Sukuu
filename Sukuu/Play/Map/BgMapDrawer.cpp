#include "stdafx.h"
#include "BgMapDrawer.h"

#include "AssetManual.h"
#include "AssetsGenerated.h"
#include "AutoTiler.h"
#include "MapGrid.h"
#include "Play/PlayScene.h"
#include "Util/TomlParametersWrapper.h"

namespace Play
{
	class MapGrid;

	void drawTileAt(
		const MapGrid& mapGrid,
		int y,
		int x,
		const Texture& texture,
		TerrainKind targetKind,
		const Array<TerrainKind>& neighborKind)
	{
		const auto& mapData = mapGrid.Data();
		constexpr uint32 tileSize{24};
		constexpr uint32 tileHalf{tileSize / 2};

		if (mapData[y][x].kind != targetKind) return;

		const AutoTileConnect connect{
			.connectL = x == 0 || neighborKind.includes(mapData[y][x - 1].kind),
			.connectT = y == 0 || neighborKind.includes(mapData[y - 1][x].kind),
			.connectR = x == mapData.size().x - 1 || neighborKind.includes(mapData[y][x + 1].kind),
			.connectB = y == mapData.size().y - 1 || neighborKind.includes(mapData[y + 1][x].kind),
		};
		const auto src = GetAutoTileSrcPoint(tileSize, connect);

		const bool isSafeRange = 0 < x && x < mapData.size().x - 1 && 0 < y && y < mapData.size().y - 1;
		const bool diagonalLT = isSafeRange && connect.connectL && connect.connectT &&
			not neighborKind.includes(mapData[y - 1][x - 1].kind);
		const bool diagonalRT = isSafeRange && connect.connectR && connect.connectT &&
			not neighborKind.includes(mapData[y - 1][x + 1].kind);
		const bool diagonalLB = isSafeRange && connect.connectL && connect.connectB &&
			not neighborKind.includes(mapData[y + 1][x - 1].kind);
		const bool diagonalRB = isSafeRange && connect.connectR && connect.connectB &&
			not neighborKind.includes(mapData[y + 1][x + 1].kind);

		if (diagonalLT || diagonalRT || diagonalLB || diagonalRB)
		{
			// 隅付き描画
			(void)texture(
					diagonalLT
						? GetAutoTileDiagonalSrc(tileSize, AutoTileDiagonal::LT)
						: src.movedBy(0, 0),
					Size{tileHalf, tileHalf})
				.draw(Point{x * tileSize, y * tileSize}.movedBy(0, 0));
			(void)texture(
					diagonalRT
						? GetAutoTileDiagonalSrc(tileSize, AutoTileDiagonal::RT)
						: src.movedBy(tileHalf, 0),
					Size{tileHalf, tileHalf})
				.draw(Point{x * tileSize, y * tileSize}.movedBy(tileHalf, 0));
			(void)texture(
					diagonalLB
						? GetAutoTileDiagonalSrc(tileSize, AutoTileDiagonal::LB)
						: src.movedBy(0, tileHalf),
					Size{tileHalf, tileHalf})
				.draw(Point{x * tileSize, y * tileSize}.movedBy(0, tileHalf));
			(void)texture(
					diagonalRB
						? GetAutoTileDiagonalSrc(tileSize, AutoTileDiagonal::RB)
						: src.movedBy(tileHalf, tileHalf),
					Size{tileHalf, tileHalf})
				.draw(Point{x * tileSize, y * tileSize}.movedBy(tileHalf, tileHalf));
		}
		else
		{
			// 通常描画
			(void)texture(src, Size{tileSize, tileSize})
				.draw(Point{x * tileSize, y * tileSize});
		}
	}

	void DrawBgMap(const MapGrid& map)
	{
		const auto inversed = (Graphics2D::GetCameraTransform() * Graphics2D::GetLocalTransform()).inverse();
		const auto mapTl = inversed.transformPoint(Vec2{0, 0}).asPoint() / CellPx_24;
		const auto mapBr = inversed.transformPoint(Scene::Size()).asPoint() / CellPx_24;
		for (int y = std::max(0, mapTl.y); y < std::min(mapBr.y + 1, map.Data().size().y); ++y)
		{
			for (int x = std::max(0, mapTl.x); x < std::min(mapBr.x + 1, map.Data().size().x); ++x)
			{
				const auto drawingPoint = Point{x, y} * CellPx_24;
				(void)TextureAsset(AssetImages::magma_tile_24x24).draw(drawingPoint);
				drawTileAt(map, y, x, TextureAsset(AssetImages::brick_stylish_24x24),
				           TerrainKind::Wall, {TerrainKind::Wall});
#ifdef _DEBUG
				const int player = PlayScene::Instance().GetPlayer().DistField()[Point{x, y}].distance;
				if (player < PlayerDistanceInfinity &&
					GetTomlParameter<bool>(U"play.debug.visualize_player_distance"))
					(void)FontAsset(AssetFonts::F24)(U"{}"_fmt(player))
						.drawAt(drawingPoint + Point{CellPx_24, CellPx_24} / 2);
#endif
			}
		}
	}
}
