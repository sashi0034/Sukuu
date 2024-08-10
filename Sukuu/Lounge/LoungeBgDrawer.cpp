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

	void Tick()
	{
		m_animTimer.Tick();
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

			(void)TextureAsset(AssetImages::bridge_tiles_24x24)(uv, Point::One() * Play::CellPx_24).draw(b.position);
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
				.draw(Arg::bottomCenter = t.movedBy(Play::CellPx_24 / 2, Play::CellPx_24/2));
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
