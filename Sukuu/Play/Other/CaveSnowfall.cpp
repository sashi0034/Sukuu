#include "stdafx.h"
#include "CaveSnowfall.h"

#include "AssetKeys.h"
#include "Assets.generated.h"
#include "Play/PlayScene.h"
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
		double time;
		Vec2 pos;
		Circular vel1;
		double velAc1;
		Circular vel2;
		double velAc2;
		double scale;
		double scalePeriod;
		double scaleBase;
		double scaleAmplitude;
	};
}

struct Play::CaveSnowfall::Impl
{
	Array<SnowfallDust> m_dusts{};
	double m_updateTimer{};

	void Update()
	{
		const double updatePeriod = 1.0 / getToml<int>(U"update_freq");
		m_updateTimer += GetDeltaTime();
		if (m_updateTimer > updatePeriod)
		{
			// ダストを更新
			m_updateTimer -= updatePeriod;
			controlDustAmount();

			for (auto&& d : m_dusts)
			{
				updateDust(d, updatePeriod);
			}
		}

		ScopedRenderStates2D sampler{BlendState::Default2D, SamplerState::ClampLinear};
		for (auto&& d : m_dusts)
		{
			drawDust(d);
		}
	}

private:
	void controlDustAmount()
	{
		// const auto inversed = (Graphics2D::GetCameraTransform() * Graphics2D::GetLocalTransform()).inverse();
		// const auto mapTl = inversed.transformPoint(Vec2{0, 0}).asPoint();
		// const auto mapBr = inversed.transformPoint(Scene::Size()).asPoint();

		const auto playerPoint =
			PlayScene::Instance().GetPlayer().CurrentPos().actualPos.movedBy(Vec2::One() * CellPx_24 / 2).asPoint();
		const auto visibleRange = (Scene::Size() / DefaultCameraScale).asPoint();
		const auto visibleRect = Rect(playerPoint - visibleRange / 2, visibleRange);

		const double visiblePadding = getToml<double>(U"visible_padding");

		for (auto&& dust : m_dusts)
		{
			if (dust.pos.x < visibleRect.leftX() - visiblePadding)
				dust.pos.x += visibleRect.size.x + visiblePadding * 2;
			if (dust.pos.x > visibleRect.rightX() + visiblePadding)
				dust.pos.x -= visibleRect.size.x + visiblePadding * 2;
			if (dust.pos.y < visibleRect.topY() - visiblePadding)
				dust.pos.y += visibleRect.size.y + visiblePadding * 2;
			if (dust.pos.y > visibleRect.bottomY() + visiblePadding)
				dust.pos.y -= visibleRect.size.y + visiblePadding * 2;
		}

		// 生成
		while (m_dusts.size() < getToml<size_t>(U"dust_amount"))
		{
			const double vr = Random(40, 120);
			const double va = Random(15_deg, 45_deg);

			const double scale = Random(0.3, 1.2);

			m_dusts.emplace_back(SnowfallDust{
				.time = 0,
				.pos = RandomPoint(visibleRect.stretched(visiblePadding)),
				.vel1 = Circular(vr, Random(0.0, Math::Pi)),
				.velAc1 = va,
				.vel2 = Circular(vr * 1.5, Random(0.0, Math::Pi)),
				.velAc2 = va / 1.5,
				.scale = 0,
				.scalePeriod = Random(1.5, 3.0),
				.scaleBase = scale,
				.scaleAmplitude = scale / 2.0
			});
		}
	}

	void drawDust(const SnowfallDust& dust) const
	{
		(void)TextureAsset(AssetImages::particle)
		      .scaled(dust.scale)
		      .drawAt(dust.pos, ColorF(1.0, getToml<double>(U"dust_alpha")));
	}

	void updateDust(SnowfallDust& dust, double dt)
	{
		dust.time += dt;
		dust.pos.y += dt * getToml<double>(U"dust_gravity");
		dust.pos += (dust.vel1.toVec2() + dust.vel2.toVec2()) * 0.5 * dt;
		dust.vel1.theta += dust.velAc1 * dt;
		dust.vel2.theta += dust.velAc2 * dt;
		dust.scale = dust.scaleBase + dust.scaleAmplitude * Periodic::Sine1_1(dust.scalePeriod, dust.time);
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
