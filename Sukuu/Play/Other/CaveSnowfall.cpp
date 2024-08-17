#include "stdafx.h"
#include "CaveSnowfall.h"

#include "Assets.generated.h"
#include "Play/PlayCore.h"

namespace
{
	struct SnowfallDust
	{
		double time;
		Vec2 oldPos;
		Vec2 pos;
		Circular vel1;
		double velAc1;
		Circular vel2;
		double velAc2;
		double oldScale;
		double scale;
		double scalePeriod;
		double scaleBase;
		double scaleAmplitude;
		Optional<Trail> trail;
	};

	struct SnowfallConditions
	{
		int amount{};
		double timescale{};
		ColorF trailColor{};
		ColorF particleColor{};
	};

	SnowfallConditions getConditions(Play::CaveSnowfallKind kind)
	{
		SnowfallConditions conditions{};

		conditions.amount = 400;
		if (kind == Play::CaveSnowfallKind::Flurries) conditions.amount = static_cast<int>(conditions.amount * 0.5);
		if (kind == Play::CaveSnowfallKind::Blizzard) conditions.amount = static_cast<int>(conditions.amount * 3.0);
		if (kind == Play::CaveSnowfallKind::Photons) conditions.amount = static_cast<int>(conditions.amount * 0.25);
		if (kind == Play::CaveSnowfallKind::Lumineer) conditions.amount = static_cast<int>(conditions.amount * 0.5);

		conditions.timescale = 1.0;
		if (kind == Play::CaveSnowfallKind::Flurries) conditions.timescale *= 5.0;
		if (kind == Play::CaveSnowfallKind::Blizzard) conditions.timescale *= 3.0;
		if (kind == Play::CaveSnowfallKind::Photons) conditions.timescale *= 0.5;
		if (kind == Play::CaveSnowfallKind::Lumineer) conditions.timescale *= 2.0;

		const bool isGolden = kind == Play::CaveSnowfallKind::Photons || kind == Play::CaveSnowfallKind::Lumineer;

		conditions.trailColor = ColorF{U"#c0c0ff"};
		if (isGolden) conditions.trailColor = ColorF{U"#c0c000"};

		conditions.particleColor = ColorF{U"#ffffff"};
		if (isGolden) conditions.particleColor = ColorF{U"#ffff80"};

		return conditions;
	}
}

struct Play::CaveSnowfall::Impl
{
	CaveSnowfallKind m_kind{};
	Array<SnowfallDust> m_dusts{};
	double m_updateTimer{};

	void Update()
	{
		const auto conditions = getConditions(m_kind);

		constexpr int updateFrequency = 15;
		const double updatePeriod = 1.0 / updateFrequency;
		m_updateTimer += GetDeltaTime();
		if (m_updateTimer > updatePeriod)
		{
			// ダストを更新
			while (m_updateTimer > updatePeriod) m_updateTimer -= updatePeriod;

			controlDustAmount(conditions.amount);

			for (auto&& d : m_dusts)
			{
				updateDust(d, updatePeriod * conditions.timescale);
			}
		}

		ScopedRenderStates2D sampler{BlendState::Default2D, SamplerState::ClampLinear};
		for (auto&& d : m_dusts)
		{
			drawDust(d, m_updateTimer / updatePeriod, conditions.trailColor, conditions.particleColor);
		}
	}

private:
	void controlDustAmount(int amount)
	{
		// const auto inversed = (Graphics2D::GetCameraTransform() * Graphics2D::GetLocalTransform()).inverse();
		// const auto mapTl = inversed.transformPoint(Vec2{0, 0}).asPoint();
		// const auto mapBr = inversed.transformPoint(Scene::Size()).asPoint();

		const auto playerPoint =
			PlayCore::Instance().GetPlayer().CurrentPos().actualPos.movedBy(Vec2::One() * CellPx_24 / 2).asPoint();
		const auto visibleRange = (Scene::Size() / DefaultCameraScale).asPoint();
		const auto visibleRect = Rect(playerPoint - visibleRange / 2, visibleRange);

		constexpr double visiblePadding = 80;

		for (auto&& dust : m_dusts)
		{
			if (dust.pos.x < visibleRect.leftX() - visiblePadding)
				dust.pos.x += visibleRect.size.x + visiblePadding * 2;
			else if (dust.pos.x > visibleRect.rightX() + visiblePadding)
				dust.pos.x -= visibleRect.size.x + visiblePadding * 2;
			else if (dust.pos.y < visibleRect.topY() - visiblePadding)
				dust.pos.y += visibleRect.size.y + visiblePadding * 2;
			else if (dust.pos.y > visibleRect.bottomY() + visiblePadding)
				dust.pos.y -= visibleRect.size.y + visiblePadding * 2;
			else continue;
			// 以下、画面外に行ってしまったときの動作

			if (dust.trail.has_value())
			{
				dust.trail = Trail();
			}
		}

		// 生成
		while (m_dusts.size() < amount)
		{
			const double vr = Random(40, 120);
			const double va = Random(15_deg, 45_deg);

			const double scale = Random(0.10, 0.15);

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
				.scaleAmplitude = scale / 2.0,
				.trail = RandomBool(0.05) ? Optional(Trail()) : none
			});
		}
	}

	static void drawDust(SnowfallDust& dust, double interpolate, const ColorF& trailColor, const ColorF& particleColor)
	{
		const auto pos = Math::Lerp(dust.oldPos, dust.pos, interpolate);

		const auto scale = Math::Lerp(dust.oldScale, dust.scale, interpolate);
		constexpr double baseAlpha = 0.5;
		constexpr double scaleFade = 5.0;
		const double alpha = baseAlpha * 1 - (scale / scaleFade);

		if (dust.trail.has_value())
		{
			dust.trail->update();
			dust.trail->add(pos,
			                ColorF{trailColor, alpha},
			                TextureAsset(AssetImages::particle).size().x * scale);
			dust.trail->draw();
		}

		(void)TextureAsset(AssetImages::particle)
		      .scaled(scale)
		      .drawAt(pos, ColorF(particleColor, alpha));
	}

	static void updateDust(SnowfallDust& dust, double dt)
	{
		dust.oldPos = dust.pos;
		dust.oldScale = dust.scale;

		dust.time += dt;
		constexpr double gravity = 20.0;
		dust.pos.y += dt * gravity;
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

	CaveSnowfall::CaveSnowfall(CaveSnowfallKind kind) : CaveSnowfall()
	{
		p_impl->m_kind = kind;
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
