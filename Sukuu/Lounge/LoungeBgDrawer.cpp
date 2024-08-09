#include "stdafx.h"
#include "LoungeBgDrawer.h"

#include "Assets.generated.h"
#include "Play/Forward.h"

namespace
{
	using namespace Lounge;
}

struct LoungeBgDrawer::Impl
{
	void DrawBack(const LoungeMapData& data, const Rect& region)
	{
		// TODO: 本当は region から最適化したほうがいい

		for (auto& m : data.manjiRegionPositions)
		{
			TextureAsset(AssetImages::manji_region).drawAt(m.movedBy(Vec2::One() * Play::CellPx_24 / 2));
		}
	}

	void DrawFront(const LoungeMapData& data)
	{
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
