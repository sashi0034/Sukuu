#include "stdafx.h"
#include "CaveSnowfall.h"

#include "AssetKeys.h"
#include "Assets.generated.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.cave_snowfall." + key);
	}

	struct SnowfallDust
	{
		Vec2 pos;
		double scale;
	};
}

struct Play::CaveSnowfall::Impl
{
	Array<SnowfallDust> m_dusts{};

	void Update()
	{
		if (m_dusts.size() < getToml<size_t>(U"dust_amount"))
		{
			m_dusts.emplace_back(SnowfallDust{
				.pos = RandomPoint(Rect(Scene::Size())),
				.scale = Random(0.5, 3.0)
			});
		}

		ScopedRenderStates2D sampler{SamplerState::ClampLinear};

		for (auto&& d : m_dusts)
		{
			updateDust(d);
		}
	}

private:
	void updateDust(SnowfallDust& dust)
	{
		(void)TextureAsset(AssetImages::particle)
		      .scaled(dust.scale)
		      .drawAt(dust.pos);
	}
};

namespace Play
{
	CaveSnowfall::CaveSnowfall() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void CaveSnowfall::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	double CaveSnowfall::OrderPriority() const
	{
		return FgEffectPriority + 1;
	}
}
