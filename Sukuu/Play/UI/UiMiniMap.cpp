#include "stdafx.h"
#include "UiMiniMap.h"

#include "Play/PlayScene.h"
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
}

struct Play::UiMiniMap::Impl
{
	Grid<ArriveFlag> m_flag{};
	RenderTexture m_renderTexture{};

	void Update()
	{
		checkRefreshMap();
		(void)m_renderTexture
			.draw(GetTomlParameter<Vec2>(U"play.ui_minimap.screen_point"));
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

		const ScopedRenderTarget2D target{m_renderTexture};
		const ScopedRenderStates2D blend{
			BlendState{
				true, Blend::One, Blend::Zero, BlendOp::Add, Blend::One, Blend::Zero, BlendOp::Max
			}
		};

		for (const auto p : step(Size{1, 1} * (vision + 1) * 2))
		{
			auto checking = playerPoint + p.movedBy(-vision, -vision);

			if (map.Data().inBounds(checking) == false) continue;
			if (m_flag[checking].isRendered) continue;
			m_flag[checking].isRendered = true;

			if (CanMovePointAt(map, checking))
			{
				// 描画
				const auto drawingTl = checking * renderCell;
				const int rc1 = renderCell; // - 1;
				Rect(drawingTl, Size{1, 1} * renderCell)
					.draw(GetTomlParameter<Color>(U"play.ui_minimap.movable_color"));

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
		p_impl->Update();
	}
}
