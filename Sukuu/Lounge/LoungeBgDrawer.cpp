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
		constexpr double swayPeriod = 1.0;
		for (int i = 0; i < m_bridgeSwayOffsets.size(); ++i)
		{
			m_bridgeSwayOffsets[i] =
				1 * Periodic::Sine1_1(
					swayPeriod, m_animTimer.Time() + swayPeriod * i / (m_bridgeSwayOffsets.size() / 2));
		}
	}

	void DrawBack(const LoungeMapData& data, const Rect& region)
	{
		Tick();

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
				.draw(Arg::bottomCenter = t.movedBy(Play::CellPx_24 / 2, Play::CellPx_24 / 2));
		}

		// 小さな木
		for (auto& t : data.smallTreePositions)
		{
			(void)TextureAsset(AssetImages::dark_tree_16x16)(m_animTimer.SliceFrames(300, 4) * 16, 0, Size::One() * 16)
				.drawAt(t.movedBy(Point::One() * Play::CellPx_24 / 2));
		}

		// 大きな木
		for (auto& t : data.bigTreePositions)
		{
			(void)TextureAsset(AssetImages::dark_tree_48x48)(m_animTimer.SliceFrames(200, 6) * 48, 0, Size::One() * 48)
				.draw(Arg::bottomCenter = t.movedBy(Play::CellPx_24 / 2, Play::CellPx_24 / 2));
		}

		// 混合植物
		for (auto& t : data.mixedNaturePositions)
		{
			(void)TextureAsset(AssetImages::mixed_nature_16x16)(t.uv, Point::One() * 16)
				.drawAt(t.position.movedBy(Point::One() * Play::CellPx_24 / 2));
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
