#include "stdafx.h"
#include "PlayScene.h"

#include "Params.h"
#include "Map/AutoTiler.h"
#include "Map/DungeonGenerator.h"
#include "Map/MapGrid.h"

namespace Play
{
	void drawTileAt(const MapGrid& mapGrid, int y, int x, const Texture& texture)
	{
		const auto& mapData = mapGrid.Data();
		constexpr uint32 tileSize{24};
		constexpr uint32 tileHalf{tileSize / 2};

		constexpr auto targetKind = TerrainKind::Wall;
		if (mapData[y][x].kind == targetKind)
		{
			const AutoTileConnect connect{
				.connectL = x == 0 || mapData[y][x - 1].kind == targetKind,
				.connectT = y == 0 || mapData[y - 1][x].kind == targetKind,
				.connectR = x == mapData.size().x - 1 || mapData[y][x + 1].kind == targetKind,
				.connectB = y == mapData.size().y - 1 || mapData[y + 1][x].kind == targetKind,
			};
			const auto src = GetAutoTileSrcPoint(tileSize, connect);

			const bool isSafeRange = 0 < x && x < mapData.size().x - 1 && 0 < y && y < mapData.size().y - 1;
			const bool diagonalLT = isSafeRange && connect.connectL && connect.connectT &&
				mapData[y - 1][x - 1].kind != targetKind;
			const bool diagonalRT = isSafeRange && connect.connectR && connect.connectT &&
				mapData[y - 1][x + 1].kind != targetKind;
			const bool diagonalLB = isSafeRange && connect.connectL && connect.connectB &&
				mapData[y + 1][x - 1].kind != targetKind;
			const bool diagonalRB = isSafeRange && connect.connectR && connect.connectB &&
				mapData[y + 1][x + 1].kind != targetKind;

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
	}

	class PlayScene::Impl
	{
	public:
		MapGrid m_map;
		Camera2D m_camera;

		void Update()
		{
			m_camera.update();
			const auto t = m_camera.createTransformer();
			const ScopedRenderStates2D sampler{SamplerState::BorderNearest};

			const auto inversed = Graphics2D::GetCameraTransform().inverse();
			const auto mapTl = inversed.transformPoint(Vec2{0, 0}).asPoint() / 24;
			const auto mapBr = inversed.transformPoint(Scene::Size()).asPoint() / 24;
			for (int y = std::max(0, mapTl.y); y < std::min(mapBr.y + 1, m_map.Data().size().y); ++y)
			{
				for (int x = std::max(0, mapTl.x); x < std::min(mapBr.x + 1, m_map.Data().size().x); ++x)
				{
					(void)TextureAsset(AssetImages::magma_tile_24x24).draw(x * 24, y * 24);
					drawTileAt(m_map, y, x, TextureAsset(AssetImages::brick_stylish_24x24));
				}
			}
		}
	};

	PlayScene::PlayScene() :
		p_impl(std::make_shared<Impl>())
	{
		p_impl->m_map = GenerateFreshDungeon(DungGenProps{
			.size = {80, 80},
			.areaDivision = 8,
		});
	}

	void PlayScene::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}
}
