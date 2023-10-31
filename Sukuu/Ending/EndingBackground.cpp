#include "stdafx.h"
#include "EndingBackground.h"

#include "AssetKeys.h"
#include "Assets.generated.h"

namespace
{
	constexpr Size mapSize{128, 128};

	constexpr int px_16 = 16;
}

struct Ending::EndingBackground::Impl
{
	void Update()
	{
		const ScopedRenderStates3D state{SamplerState::BorderNearest};

		for (const auto p : step({}, mapSize, {4, 4}))
		{
			(void)TextureAsset(AssetImages::grass_tile_64x64).draw(p * px_16);
		}
	}
};

namespace Ending
{
	EndingBackground::EndingBackground() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void EndingBackground::Init()
	{
	}

	void EndingBackground::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}
}
