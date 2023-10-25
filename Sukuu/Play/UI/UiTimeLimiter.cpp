#include "stdafx.h"
#include "UiTimeLimiter.h"

#include "AssetKeys.h"
#include "Play/Other/TimeLimiter.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.ui_time_limiter." + key);
	}

	constexpr int arcAreaSeconds = 60;
}

struct Play::UiTimeLimiter::Impl
{
	TimeLimiterData m_data{};
	double m_fallingDamage{};
	double m_shadowFallingDamage{};
	ActorBase m_damaging{};

	void Update()
	{
		m_data.remainingTime -= GetDeltaTime();
		if (m_data.remainingTime < 0) m_data.remainingTime = 0;

		if (m_shadowFallingDamage > 0)
		{
			// ダメージエフェクトを進める
			const double delta = m_shadowFallingDamage - m_fallingDamage;
			m_shadowFallingDamage = m_fallingDamage;
			m_data.remainingTime = std::max(m_data.remainingTime - delta, 0.0);
		}

		drawUi();
	}

	void Damage(ActorBase& self, double time)
	{
		m_fallingDamage += time;
		m_shadowFallingDamage = m_fallingDamage;
		m_damaging.Kill();
		m_damaging = static_cast<ActorBase>(AnimateEasing<EaseInQuint>(self, &m_fallingDamage, 0.0, 1.0));
	}

private:
	void drawUi() const
	{
		const auto center = Scene::Size().x0() + getToml<Point>(U"circle_center");
		const auto mainColor = getToml<Color>(U"main_color");
		const auto emptyColor = getToml<Color>(U"empty_color");
		const auto damageColor = getToml<Color>(U"damage_color");
		const double emptyThickness = getToml<double>(U"empty_thickness");
		const int circleRadius = getToml<int>(U"circle_radius");
		const auto circle = Circle(center, circleRadius);
		const int circleOuter = getToml<int>(U"circle_outer");

		(void)circle.scaled(getToml<double>(U"bg_back_scale"))
		            .draw(getToml<Color>(U"bg_back_color"))
		            .drawShadow({0, 4}, 8, 2);
		(void)circle.scaled(getToml<double>(U"back_scale"))
		            .draw(getToml<Color>(U"back_color"))
		            .drawShadow({0, 4}, 8, 2);

		const double arcRate = Math::Clamp((m_data.remainingTime - m_fallingDamage) / arcAreaSeconds, 0.0, 1.0);
		const double arcAmount = 270_deg * arcRate;

		// 空部分
		(void)circle.drawArc(0_deg, 270_deg, -emptyThickness, circleOuter + emptyThickness, emptyColor);
		if (m_fallingDamage > 0)
		{
			// ダメージ部分
			const double damageArcAmount = 270_deg * Math::Clamp(m_data.remainingTime / arcAreaSeconds, 0.0, 1.0);
			(void)circle.drawArc(270_deg - damageArcAmount, damageArcAmount, 0, circleOuter, damageColor);
		}
		// 残量
		(void)circle.drawArc(270_deg - arcAmount, arcAmount, 0, circleOuter, mainColor);

		// アイコン描画
		(void)TextureAsset(U"⏳").resized(getToml<int>(U"icon_size")).drawAt(center);

		FontAsset(AssetKeys::RocknRoll24)(U"のこり {}"_fmt(static_cast<int>(m_data.remainingTime)))
			.draw(Arg::rightCenter = center.movedBy(getToml<Vec2>(U"text_offset")));

		// 長方形部分描画
		const auto rectBt = center.movedBy(0, -circleRadius - circleOuter);
		const auto rectH = circleOuter;
		const double rectUnit = getToml<double>(U"rect_unit_width");
		const double rectMaxRate = (m_data.maxTime - arcAreaSeconds) / arcAreaSeconds;
		const double rectRate = (m_data.remainingTime - m_fallingDamage - arcAreaSeconds) / arcAreaSeconds;
		const auto rectMaxW = rectUnit * rectMaxRate;
		const auto rectW = rectUnit * rectRate;
		if (rectMaxRate > 0)
			// 空部分
			(void)Rect(rectBt.movedBy(-rectMaxW, -emptyThickness), SizeF{rectMaxW, rectH}.asPoint())
				.draw(emptyColor);
		if (m_fallingDamage > 0 && m_data.remainingTime > arcAreaSeconds)
		{
			// ダメージ部分
			const auto damageW = rectUnit * (m_data.remainingTime - arcAreaSeconds) / arcAreaSeconds;
			(void)Rect(rectBt.movedBy(-damageW, 0), SizeF{damageW, rectH}.asPoint())
				.draw(damageColor);
		}
		if (rectRate > 0)
			// 残量
			(void)Rect(rectBt.movedBy(-rectW, 0), SizeF{rectW, rectH}.asPoint())
				.draw(mainColor);
	}
};

namespace Play
{
	UiTimeLimiter::UiTimeLimiter() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void UiTimeLimiter::Init(const TimeLimiterData& data)
	{
		p_impl->m_data = data;
	}

	void UiTimeLimiter::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	void UiTimeLimiter::Damage(double time)
	{
		p_impl->Damage(*this, time);
	}

	const TimeLimiterData& UiTimeLimiter::GetData() const
	{
		return p_impl->m_data;
	}
}
