#include "stdafx.h"
#include "UiTimeLimiter.h"

#include "AssetKeys.h"
#include "Play/Other/TimeLimiter.h"
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

	void Update()
	{
		m_data.remainingTime -= GetDeltaTime();
		if (m_data.remainingTime < 0) m_data.remainingTime = 0;

		drawUi();
	}

private:
	void drawUi() const
	{
		const auto center = Scene::Size().x0() + getToml<Point>(U"circle_center");
		const auto mainColor = getToml<Color>(U"main_color");
		const auto emptyColor = getToml<Color>(U"empty_color");
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

		const double arcRate = std::min(m_data.remainingTime / arcAreaSeconds, 1.0);
		const double arcAmount = 270_deg * arcRate;
		(void)circle
		      // 空部分
		      .drawArc(0_deg, 270_deg, -emptyThickness, circleOuter + emptyThickness, emptyColor)
		      // 残量
		      .drawArc(270_deg - arcAmount, arcAmount, 0, circleOuter, mainColor);

		// アイコン描画
		(void)TextureAsset(U"⏳").resized(getToml<int>(U"icon_size")).drawAt(center);

		FontAsset(AssetKeys::RocknRoll24)(U"のこり {}"_fmt(static_cast<int>(m_data.remainingTime)))
			.draw(Arg::rightCenter = center.movedBy(getToml<Vec2>(U"text_offset")));

		// 長方形部分描画
		const auto rectBt = center.movedBy(0, -circleRadius - circleOuter);
		const auto rectH = circleOuter;
		const double rectUnit = getToml<double>(U"rect_unit_width");
		const double rectMaxRate = (m_data.maxTime - arcAreaSeconds) / arcAreaSeconds;
		const double rectRate = (m_data.remainingTime - arcAreaSeconds) / arcAreaSeconds;
		const auto rectMaxW = rectUnit * rectMaxRate;
		const auto rectW = rectUnit * (m_data.remainingTime - arcAreaSeconds) / arcAreaSeconds;
		if (rectMaxRate > 0)
			// 空部分
			(void)Rect(rectBt.movedBy(-rectMaxW, -emptyThickness), SizeF{rectMaxW, rectH}.asPoint())
				.draw(emptyColor);
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

	const TimeLimiterData& UiTimeLimiter::GetData() const
	{
		return p_impl->m_data;
	}
}
