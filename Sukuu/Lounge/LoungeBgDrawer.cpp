#include "stdafx.h"
#include "LoungeBgDrawer.h"

#include "Assets.generated.h"
#include "Play/Forward.h"
#include "Play/Chara/CharaUtil.h"

namespace
{
	using namespace Lounge;
}

struct LoungeBgDrawer::Impl
{
	Play::AnimTimer m_animTimer{};

	std::array<double, 8> m_bridgeSwayOffsets{};

	void Tick()
	{
		m_animTimer.Tick();

		// 橋の揺れのテーブル更新
		for (int i = 0; i < m_bridgeSwayOffsets.size(); ++i)
		{
			constexpr double swayPeriod = 2.0;
			m_bridgeSwayOffsets[i] =
				1 * Periodic::Sine1_1(
					swayPeriod, m_animTimer.Time() + swayPeriod * i / (m_bridgeSwayOffsets.size() / 2));
		}
	}

	void renderWeeds(const Rect& region)
	{
		// 雑草
		constexpr std::array<int, 25> weedsPatterns = {
			1, 2, 3, 4, 3,
			2, 0, 2, 1, 4,
			4, 2, 0, 3, 5,
			5, 4, 2, 4, 3,
			2, 5, 4, 3, 5,
		};

		const auto weedsTexture = TextureAsset(AssetImages::dark_weeds_16x16);
		const int s4 = m_animTimer.SliceFrames(250, 4);

		for (int x = region.x; x < region.x + region.w; ++x)
		{
			for (int y = region.y; y < region.y + region.h; ++y)
			{
				const int modX = ((x % 5) + 5) % 5;
				const int modY = ((y % 5) + 5) % 5;
				const int index = modX + modY * 5;
				const int pattern = weedsPatterns[index];
				const int f4 = (x + y + 4 + s4) % 4;

				const Vec2 pos = (Vec2{x, y} * Play::CellPx_24).movedBy(4, 4);
				switch (pattern)
				{
				case 0:
					(void)weedsTexture(Point{0, 0} * 16, Size::One() * 16).draw(pos);
					break;
				case 1:
					(void)weedsTexture(Point{1, 0} * 16, Size::One() * 16).draw(pos);
					break;
				case 2:
					(void)weedsTexture(Point{2, 0} * 16, Size::One() * 16).draw(pos);
					break;
				case 3:
					(void)weedsTexture(Point{3, 0} * 16, Size::One() * 16).draw(pos);
					break;
				case 4:
					(void)weedsTexture(Point{f4, 1} * 16, Size::One() * 16).draw(pos);
					break;
				case 5:
					(void)weedsTexture(Point{f4, 2} * 16, Size::One() * 16).draw(pos);
					break;
				default: break;
				}
			}
		}
	}

	void DrawBack(const LoungeMapData& data, const Rect& region)
	{
		Tick();

		{
			// 背景色
			Transformer2D t2d{Mat3x2::Identity(), Transformer2D::Target::SetLocal};
			Rect(Scene::Size()).draw(ColorF{0.10, 0.175, 0.25});
		}

		// 雑草
		renderWeeds(region);

		// TODO: 本当は region から最適化したほうがいい

		// 卍
		for (auto& m : data.manjiRegionPositions)
		{
			TextureAsset(AssetImages::manji_region).drawAt(m.movedBy(Vec2::One() * Play::CellPx_24 / 2));
		}

		// 橋
		for (auto& b : data.bridgePositions)
		{
			Point uv{};
			if (b.kind == LoungeBridgeKind::Hl) uv = Point{0, 0};
			else if (b.kind == LoungeBridgeKind::Hc) uv = Point{1, 0};
			else if (b.kind == LoungeBridgeKind::Hr) uv = Point{2, 0};
			else if (b.kind == LoungeBridgeKind::Vt) uv = Point{0, 1};
			else if (b.kind == LoungeBridgeKind::Vm) uv = Point{1, 1};
			else if (b.kind == LoungeBridgeKind::Vb) uv = Point{2, 1};
			uv *= Play::CellPx_24;

			const int index0 = (b.hash * 2);
			if (uv.y == 0)
			{
				// Horizontal
				const Vec2 offset0 = Vec2{0, 1} * m_bridgeSwayOffsets[index0 % m_bridgeSwayOffsets.size()];
				const Vec2 offset1 = Vec2{0, 1} * m_bridgeSwayOffsets[(index0 + 1) % m_bridgeSwayOffsets.size()];
				(void)TextureAsset(AssetImages::bridge_tiles_24x24)(uv, {12, 24})
					.draw(b.position.movedBy(offset0));
				(void)TextureAsset(AssetImages::bridge_tiles_24x24)(uv.movedBy(12, 0), {12, 24})
					.draw(b.position.movedBy(12, 0).movedBy(offset1));
			}
			else
			{
				// Vertical
				const Vec2 offset0 = Vec2{1, 0} * m_bridgeSwayOffsets[index0 % m_bridgeSwayOffsets.size()];
				const Vec2 offset1 = Vec2{1, 0} * m_bridgeSwayOffsets[(index0 + 1) % m_bridgeSwayOffsets.size()];
				(void)TextureAsset(AssetImages::bridge_tiles_24x24)(uv, {24, 12})
					.draw(b.position.movedBy(offset0));
				(void)TextureAsset(AssetImages::bridge_tiles_24x24)(uv.movedBy(0, 12), {24, 12})
					.draw(b.position.movedBy(0, 12).movedBy(offset1));
			}
		}
	}

	void DrawFront(const LoungeMapData& data)
	{
		// 鳥居
		for (auto& t : data.toriiPositions)
		{
			TextureAsset(AssetImages::torii_120x120)
				.draw(Arg::bottomCenter = t.movedBy(Play::CellPx_24 / 2, Play::CellPx_24 / 2));
		}

		// 灯籠
		for (auto& t : data.tourouPositions)
		{
			TextureAsset(AssetImages::tourou_24x48)
				.draw(Arg::bottomCenter = t.movedBy(Play::CellPx_24 / 2, Play::CellPx_24 * 3 / 4));
		}

		// 大きな木
		for (auto& t : data.treePositions)
		{
			(void)TextureAsset(AssetImages::pink_tree_48x48)(m_animTimer.SliceFrames(200, 6) * 48, 0, Size::One() * 48)
				.draw(Arg::bottomCenter = t.movedBy(Play::CellPx_24 / 2, Play::CellPx_24 / 2));
		}

		{
			// 視界を暗めにする
			Transformer2D t2d{Mat3x2::Identity(), Transformer2D::Target::SetLocal};
			// double root2 = std::sqrt(2.0);
			// Ellipse(Scene::Center(), Scene::Center().x * root2, Scene::Center().y * root2)
			// 	.draw(ColorF{0, 0, 0, 0}, ColorF{0.11, 0.19, 0.27});

			// ユークリッド距離のほうが半径として正しいが、暗すぎたのであえてマンハッタン距離を採用
			Circle(Scene::Center(), Scene::Center().manhattanLength())
				.draw(ColorF{0, 0, 0, 0}, ColorF{0.11, 0.19, 0.27});
		}
	}
};

namespace Lounge
{
	LoungeBgDrawer::LoungeBgDrawer() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void LoungeBgDrawer::DrawBack(const LoungeMapData& data, const Rect& region)
	{
		p_impl->DrawBack(data, region);
	}

	void LoungeBgDrawer::DrawFront(const LoungeMapData& data)
	{
		p_impl->DrawFront(data);
	}
}
