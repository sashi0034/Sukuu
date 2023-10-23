#include "stdafx.h"
#include "BgMapDrawer.h"

#include "AssetKeys.h"
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
		constexpr uint32 tileHalf{CellPx_24 / 2};

		if (mapData[y][x].kind != targetKind) return;

		const AutoTileConnect connect{
			.connectL = x == 0 || neighborKind.includes(mapData[y][x - 1].kind),
			.connectT = y == 0 || neighborKind.includes(mapData[y - 1][x].kind),
			.connectR = x == mapData.size().x - 1 || neighborKind.includes(mapData[y][x + 1].kind),
			.connectB = y == mapData.size().y - 1 || neighborKind.includes(mapData[y + 1][x].kind),
		};
		const auto src = GetAutoTileSrcPoint(CellPx_24, connect);

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
						? GetAutoTileDiagonalSrc(CellPx_24, AutoTileDiagonal::LT)
						: src.movedBy(0, 0),
					Size{tileHalf, tileHalf})
				.draw(Point{x * CellPx_24, y * CellPx_24}.movedBy(0, 0));
			(void)texture(
					diagonalRT
						? GetAutoTileDiagonalSrc(CellPx_24, AutoTileDiagonal::RT)
						: src.movedBy(tileHalf, 0),
					Size{tileHalf, tileHalf})
				.draw(Point{x * CellPx_24, y * CellPx_24}.movedBy(tileHalf, 0));
			(void)texture(
					diagonalLB
						? GetAutoTileDiagonalSrc(CellPx_24, AutoTileDiagonal::LB)
						: src.movedBy(0, tileHalf),
					Size{tileHalf, tileHalf})
				.draw(Point{x * CellPx_24, y * CellPx_24}.movedBy(0, tileHalf));
			(void)texture(
					diagonalRB
						? GetAutoTileDiagonalSrc(CellPx_24, AutoTileDiagonal::RB)
						: src.movedBy(tileHalf, tileHalf),
					Size{tileHalf, tileHalf})
				.draw(Point{x * CellPx_24, y * CellPx_24}.movedBy(tileHalf, tileHalf));
		}
		else
		{
			// 通常描画
			(void)texture(src, Size{CellPx_24, CellPx_24})
				.draw(Point{x * CellPx_24, y * CellPx_24});
		}
	}

	void drawGimmickAt(
		const GimmickGrid& gimmickGrid,
		const Point& point)
	{
		switch (gimmickGrid[point])
		{
		case GimmickKind::None:
			break;
		case GimmickKind::Stairs:
			(void)TextureAsset(AssetImages::stairs_24x24).draw(point * CellPx_24);
			break;
		default: ;
		}
	}

	void DrawBgMap(const PlayScene& scene)
	{
		const auto inversed = (Graphics2D::GetCameraTransform() * Graphics2D::GetLocalTransform()).inverse();
		const auto mapTl = inversed.transformPoint(Vec2{0, 0}).asPoint() / CellPx_24;
		const auto mapBr = inversed.transformPoint(Scene::Size()).asPoint() / CellPx_24;
		auto&& map = scene.GetMap();
		auto&& gimmick = scene.GetGimmick();
		for (int y = std::max(0, mapTl.y); y < std::min(mapBr.y + 1, map.Data().size().y); ++y)
		{
			for (int x = std::max(0, mapTl.x); x < std::min(mapBr.x + 1, map.Data().size().x); ++x)
			{
				// BG描画
				const auto drawingPoint = Point{x, y} * CellPx_24;
				(void)TextureAsset(AssetImages::magma_tile_24x24).draw(drawingPoint);
				drawTileAt(map, y, x, TextureAsset(AssetImages::brick_stylish_24x24),
				           TerrainKind::Wall, {TerrainKind::Wall});

				// ギミック描画
				drawGimmickAt(gimmick, {x, y});

#ifdef _DEBUG
				const int player = scene.GetPlayer().DistField()[Point{x, y}].distance;
				if (player < PlayerDistanceInfinity &&
					GetTomlParameter<bool>(U"play.debug.visualize_player_distance"))
					(void)FontAsset(AssetKeys::F24)(U"{}"_fmt(player))
						.drawAt(drawingPoint + Point{CellPx_24, CellPx_24} / 2);
#endif
			}
		}
	}
}
