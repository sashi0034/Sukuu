#include "stdafx.h"
#include "UiMiniMap.h"

#include "Play/PlayScene.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	struct ArriveFlag
	{
		bool isArrived{};
		bool isRendered{};
	};

	using namespace Util;

	int getRenderCellSize()
	{
		return GetTomlParameter<int>(U"play.ui_minimap.render_cell");
	}

	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.ui_minimap." + key);
	}
}

struct Play::UiMiniMap::Impl
{
	Grid<ArriveFlag> m_flag{};
	RenderTexture m_renderTexture{};
	Array<Point> m_discoveredItemPoints{};
	bool m_hasExploreStairsAndAllItems{};
	Vec2 m_drawScale{1, 1};
	bool m_showEnemies{};

	void Update()
	{
		checkRefreshMap();
		Transformer2D transform{
			Mat3x2::Translate(getToml<Vec2>(U"screen_point"))
			.scaled(m_drawScale)
		};

		// マップ描画
		(void)m_renderTexture.draw();

		const int rc = getRenderCellSize();

		auto&& scene = PlayScene::Instance();

		if (static_cast<int>(Scene::Time() * 10) % 5 >= 2)
		{
			// プレイヤー描画
			const auto playerPos =
				scene.GetPlayer().CurrentPos().actualPos.movedBy(Point{CellPx_24, CellPx_24} / 2) / CellPx_24;
			(void)Circle(playerPos * rc, rc / 2).draw(getToml<Color>(U"player_color"));
		}

		const auto itemColor = getToml<Color>(U"item_color");
		for (int i = m_discoveredItemPoints.size() - 1; i >= 0; i--)
		{
			// アイテム描画
			auto&& p = m_discoveredItemPoints[i];
			if (scene.GetGimmick()[p] == GimmickKind::None)
			{
				m_discoveredItemPoints.remove_at(i);
			}
			(void)Circle((p * rc).movedBy(rc / 2, rc / 2), rc / 2).draw(itemColor);
		}

		if (m_showEnemies)
		{
			// エネミー描画
			const auto enemyColor = getToml<Color>(U"enemy_color");
			PlayScene::Instance().GetEnemies().ForEach([&](auto&& enemy)
			{
				auto enemyPos = enemy.Pos().actualPos.movedBy(Point{CellPx_24, CellPx_24} / 2) / CellPx_24;
				(void)Circle(enemyPos * rc, rc / 2).draw(enemyColor);
			});
		}
	}

	bool SpotStairsAndAllItems(ActorView self)
	{
		// 階段とアイテムを探して全て表示
		auto&& gimmick = PlayScene::Instance().GetGimmick();
		if (m_hasExploreStairsAndAllItems) return false;
		const int renderCell = getRenderCellSize();
		const auto scoped = scopeRenderMinimap();
		for (auto&& p : step(gimmick.size()))
		{
			if (m_flag[p].isRendered) continue;
			if (gimmick[p] == GimmickKind::Stairs ||
				GimmickToItem(gimmick[p]) != ConsumableItem::None)
			{
				inquireStairsAndItemsAt(gimmick, renderCell, p, p * renderCell);
			}
		}
		m_hasExploreStairsAndAllItems = true;

		// マップの拡縮アニメーション
		AnimateShake(self);

		return true;
	}

	void AnimateShake(ActorView self)
	{
		StartCoro(self, [this, self](YieldExtended yield)
		{
			yield.WaitForDead(AnimateEasing<EaseInBack>(self, &m_drawScale, {1, 0}, 0.3));
			AnimateEasing<EaseOutBack>(self, &m_drawScale, {1, 1}, 0.3);
		});
	}

private:
	void checkRefreshMap()
	{
		auto&& player = PlayScene::Instance().GetPlayer();
		const auto playerPoint = player.CurrentPoint();
		if (player.DistField().IsPlayerExistAt(playerPoint) == false) return;
		if (m_flag[playerPoint].isArrived) return;
		m_flag[playerPoint].isArrived = true;

		auto&& map = PlayScene::Instance().GetMap();

		const int vision = GetTomlParameter<int>(U"play.ui_minimap.vision_render");
		const int renderCell = getRenderCellSize();

		const auto scoped = scopeRenderMinimap();

		// 新たな範囲を調査
		for (const auto p : step(Size{1, 1} * (vision + 1) * 2))
		{
			auto checking = playerPoint + p.movedBy(-vision, -vision);

			if (map.Data().inBounds(checking) == false) continue;
			if (m_flag[checking].isRendered) continue;
			m_flag[checking].isRendered = true;

			if (CanMovePointAt(map, checking))
			{
				drawCellAt(map, renderCell, checking);
			}
		}
	}

	std::pair<ScopedRenderTarget2D, ScopedRenderStates2D> scopeRenderMinimap()
	{
		return {
			ScopedRenderTarget2D{m_renderTexture},
			ScopedRenderStates2D{
				BlendState{
					true, Blend::One, Blend::Zero, BlendOp::Add, Blend::One, Blend::Zero, BlendOp::Max
				}
			}
		};
	}

	void drawCellAt(const MapGrid& map, int renderCell, const Point& checking)
	{
		auto&& gimmick = PlayScene::Instance().GetGimmick();

		// 描画
		const Point drawingTl = checking * renderCell;
		const int rc1 = renderCell;
		Rect(drawingTl, Size{1, 1} * renderCell)
			.draw(GetTomlParameter<Color>(U"play.ui_minimap.movable_color"));

		// 階段とアイテム描画
		inquireStairsAndItemsAt(gimmick, renderCell, checking, drawingTl);

		// 境界を描画
		const auto borderColor = GetTomlParameter<Color>(U"play.ui_minimap.border_color");
		if (CanMovePointTo(map, checking, Dir4::Right) == false)
			(void)Line(drawingTl.movedBy(rc1, 0), drawingTl.movedBy(rc1, rc1)).draw(borderColor);
		if (CanMovePointTo(map, checking, Dir4::Up) == false)
			(void)Line(drawingTl, drawingTl.movedBy(rc1, 0)).draw(borderColor);
		if (CanMovePointTo(map, checking, Dir4::Left) == false)
			(void)Line(drawingTl, drawingTl.movedBy(0, rc1)).draw(borderColor);
		if (CanMovePointTo(map, checking, Dir4::Down) == false)
			(void)Line(drawingTl.movedBy(0, rc1), drawingTl.movedBy(rc1, rc1)).draw(borderColor);
	}

	void inquireStairsAndItemsAt(
		const GimmickGrid& gimmick,
		int renderCell,
		const Point& checking,
		const Point& drawingTl)
	{
		if (gimmick[checking] == GimmickKind::Stairs)
		{
			// 階段を描画
			Rect(drawingTl, (Size{1, 1} * renderCell).movedBy(-2, -2))
				.drawFrame(2, GetTomlParameter<Color>(U"play.ui_minimap.stairs_color"));
		}
		if (GimmickToItem(gimmick[checking]) != ConsumableItem::None)
		{
			// アイテム発見
			m_discoveredItemPoints.push_back(checking);
		}
	}
};

namespace Play
{
	UiMiniMap::UiMiniMap() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void UiMiniMap::Init(const Size& mapSize)
	{
		p_impl->m_flag.resize(mapSize);
		p_impl->m_renderTexture = RenderTexture(mapSize * getRenderCellSize(), ColorF{0, 0});
	}

	void UiMiniMap::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	bool UiMiniMap::SpotStairsAndAllItems()
	{
		return p_impl->SpotStairsAndAllItems(*this);
	}

	void UiMiniMap::SetShowEnemies(bool isShow)
	{
		p_impl->AnimateShake(*this);
		p_impl->m_showEnemies = isShow;
	}

	bool UiMiniMap::IsShowEnemies() const
	{
		return p_impl->m_showEnemies;
	}
}
