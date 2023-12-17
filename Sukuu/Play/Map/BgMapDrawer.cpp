#include "stdafx.h"
#include "BgMapDrawer.h"

#include "AssetKeys.h"
#include "Assets.generated.h"
#include "AutoTiler.h"
#include "MapGrid.h"
#include "Play/PlayCore.h"
#include "Play/Chara/CharaUtil.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	using namespace Play;

	void drawTileAt(
		const MapGrid& mapGrid,
		int x,
		int y,
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
}

struct Play::BgMapDrawer::Impl
{
	AnimTimer m_animTimer{};
	Array<std::function<void()>> m_postDraws{};
	std::function<ScopedCustomShader2D(double t)> m_bgShader = [](auto) { return ScopedCustomShader2D{}; };

	void DrawGimmickAt(
		const GimmickGrid& gimmickGrid,
		const Point& point,
		Array<std::function<void()>>& postDraws) const
	{
		switch (gimmickGrid[point])
		{
		case GimmickKind::None:
			break;
		case GimmickKind::Stairs:
			(void)TextureAsset(AssetImages::stairs_24x24).draw(point * CellPx_24);
			break;
		case GimmickKind::Item_Wing:
			drawGimmickAt(point, AssetImages::wing_16x16, 16, 4);
			break;
		case GimmickKind::Item_Helmet:
			drawGimmickAt(point, AssetImages::helmet_16x16, 16, 4);
			break;
		case GimmickKind::Item_Pin:
			drawGimmickAt(point, AssetImages::pin_16x16, 16, 3);
			break;
		case GimmickKind::Item_Mine:
			drawGimmickAt(point, AssetImages::mine_flower_16x16, 16, 6, {0, 1});
			break;
		case GimmickKind::Item_LightBulb:
			drawGimmickAt(point, AssetImages::light_bulb_16x16, 16, 3);
			break;
		case GimmickKind::Item_Magnet:
			drawGimmickAt(point, AssetImages::magnet_16x16, 16, 3);
			break;
		case GimmickKind::Item_Bookmark:
			drawGimmickAt(point, AssetImages::bookmark_16x16, 16, 3);
			break;
		case GimmickKind::Item_Explorer:
			drawGimmickAt(point, AssetImages::folder_16x16, 16, 4);
			break;
		case GimmickKind::Item_Grave:
			drawGimmickAt(point, AssetImages::grave_16x16, 16, 4);
			break;
		case GimmickKind::Item_Sun:
			drawGimmickAt(point, AssetImages::omeme_patchouli_sun_16x16, 16, 2);
			break;
		case GimmickKind::Item_Tube:
			drawGimmickAt(point, AssetImages::test_tube_16x16, 16, 3);
			break;
		case GimmickKind::Item_Solt:
			drawGimmickAt(point, AssetImages::solt_16x16, 16, 3);
			break;
		case GimmickKind::Item_Rocket:
			drawGimmickAt(point, AssetImages::rocket_16x16, 16, 4);
			break;
		case GimmickKind::SemiItem_Hourglass:
			drawGimmickAt(point, AssetImages::hourglass_16x16, 16, 3);
			break;
		case GimmickKind::SemiItem_Vessel:
			drawGimmickAt(point, AssetImages::heart_vessel_24x24, 24, 3);
			break;
		case GimmickKind::Arrow_right:
			(void)TextureAsset(AssetImages::arrow_24x24)(Point{1, 0} * 24, 24, 24).draw(point * CellPx_24);
			break;
		case GimmickKind::Arrow_up:
			(void)TextureAsset(AssetImages::arrow_24x24)(Point{0, 0} * 24, 24, 24).draw(point * CellPx_24);
			break;
		case GimmickKind::Arrow_left:
			(void)TextureAsset(AssetImages::arrow_24x24)(Point{3, 0} * 24, 24, 24).draw(point * CellPx_24);
			break;
		case GimmickKind::Arrow_down:
			(void)TextureAsset(AssetImages::arrow_24x24)(Point{2, 0} * 24, 24, 24).draw(point * CellPx_24);
			break;
		case GimmickKind::Tree_small:
			(void)TextureAsset(AssetImages::dark_tree_16x16)(Point{m_animTimer.SliceFrames(250, 4), 0} * 16, {16, 16})
				.draw(point * CellPx_24 + GetCharacterCellPadding({16, 16}));
			break;
		case GimmickKind::Tree_large:
			postDraws.emplace_back([this, point]()
			{
				(void)TextureAsset(AssetImages::dark_tree_48x48)(
						Point{m_animTimer.SliceFrames(200, 6), 0} * 48, {48, 48})
					.draw(point * CellPx_24 + GetCharacterCellPadding({48, 48}));
			});
			break;
		default: ;
		}
	}

private:
	void drawGimmickAt(
		const Point& point, AssetNameView name, int cellSize, int numFrames, const Point& cellOffset = {}) const
	{
		(void)TextureAsset(name)(
				Rect{
					Point{cellSize, 0} * m_animTimer.SliceFrames(250, numFrames) + cellOffset * cellSize,
					{cellSize, cellSize}
				})
			.draw(point * CellPx_24 + GetItemCellPadding({cellSize, cellSize}));
	}
};

namespace Play
{
	BgMapDrawer::BgMapDrawer() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void DrawBgMapTileAt(const MapGrid& map, int x, int y)
	{
		drawTileAt(map, x, y, TextureAsset(AssetImages::magma_tile_24x24),
		           TerrainKind::Pathway, {TerrainKind::Floor, TerrainKind::Pathway});
		drawTileAt(map, x, y, TextureAsset(AssetImages::magma_tile_24x24),
		           TerrainKind::Floor, {TerrainKind::Floor, TerrainKind::Pathway});

		drawTileAt(map, x, y, TextureAsset(AssetImages::brick_stylish_24x24),
		           TerrainKind::Wall, {TerrainKind::Wall});
	}

	void BgMapDrawer::UpdateDraw()
	{
		auto&& scene = PlayCore::Instance();
		p_impl->m_animTimer.Tick();

		const auto inversed = (Graphics2D::GetCameraTransform() * Graphics2D::GetLocalTransform()).inverse();
		const auto mapTl = inversed.transformPoint(Vec2{0, 0}).asPoint() / CellPx_24;
		const auto mapBr = inversed.transformPoint(Scene::Size()).asPoint() / CellPx_24;
		auto&& map = scene.GetMap();

		[&]()
		{
			// BG描画
			const ScopedCustomShader2D shader{p_impl->m_bgShader(p_impl->m_animTimer.Time())};
			const auto outTexture = TextureAsset((AssetImages::brick_stylish_24x24))(Point{2, 1} * 24, {24, 24});
			for (int y = mapTl.y - 1; y < mapBr.y + 1; ++y)
			{
				for (int x = mapTl.x - 1; x < mapBr.x + 1; ++x)
				{
					if (map.Data().inBounds(y, x) == false)
					{
						// マップ範囲外
						outTexture.draw(Point{x, y} * CellPx_24, Palette::Lightblue);
						continue;
					}

					DrawBgMapTileAt(map, x, y);
				}
			}
		}();

		auto&& gimmick = scene.GetGimmick();;
		// 木の描画のため縦方向は若干余裕がある
		for (int y = mapTl.y - 1; y < mapBr.y + 2; ++y)
		{
			for (int x = mapTl.x - 1; x < mapBr.x + 1; ++x)
			{
				if (map.Data().inBounds(y, x) == false) continue;

				// ギミック描画
				p_impl->DrawGimmickAt(gimmick, {x, y}, p_impl->m_postDraws);

#if _DEBUG
				const int player = scene.GetPlayer().DistField()[Point{x, y}].distance;
				if (player < PlayerDistanceInfinity &&
					GetTomlParameter<bool>(U"debug.visualize_player_distance"))
					(void)FontAsset(AssetKeys::RocknRoll_24_Bitmap)(U"{}"_fmt(player))
						.drawAt(Point{x, y} * CellPx_24 + Point{CellPx_24, CellPx_24} / 2);
#endif
			}
		}
	}

	void BgMapDrawer::PostDraw()
	{
		for (auto&& d : p_impl->m_postDraws)
		{
			d();
		}
		p_impl->m_postDraws.clear();
	}

	void BgMapDrawer::SetBgShader(const std::function<ScopedCustomShader2D(double t)>& shader)
	{
		p_impl->m_bgShader = shader;
	}
}
