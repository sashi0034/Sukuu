﻿#include "stdafx.h"
#include "UiTimeLimiter.h"

#include "AssetKeys.h"
#include "Gm/LocalizedTextDatabase.h"
#include "Play/PlayingUra.h"
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

	Vec2 getDefaultPos()
	{
		return Scene::Size().x0() + getToml<Point>(U"circle_center");
	}

	constexpr int arcAreaSeconds = 60;
}

struct Play::UiTimeLimiter::Impl
{
	bool m_countEnabled{true};
	bool m_immortal{};
	TimeLimiterData m_data{};
	double m_outsideDelta{};
	double m_shadowOutsideDelta{};
	ActorWeak m_outsider{};
	bool m_fromEnemyDamage{};

	// 見かけ上の制限時間がゼロにする
	double m_misrepresentedAsIfZero{};

	Vec2 m_pos{};
	Vec2 m_targetPos{};

	void Update()
	{
		if (m_countEnabled)
		{
			// 実時間で時を進めることにする
			const double dt = Scene::DeltaTime();
			const double slowCoefficient = IsPlayingUra()
				                               ? 1.0
				                               : 0.5;
			m_data.remainingTime -= dt * slowCoefficient;
			if (m_data.remainingTime < 0) m_data.remainingTime = 0;
		}

		if (m_shadowOutsideDelta != 0)
		{
			// 回復またはダメージエフェクトを進める
			const double delta = m_shadowOutsideDelta - m_outsideDelta;
			m_shadowOutsideDelta = m_outsideDelta;
			if (m_shadowOutsideDelta > 0)
				m_data.remainingTime = std::max(m_data.remainingTime - delta, 0.0);
			else
				m_data.remainingTime = std::min(m_data.remainingTime - delta, m_data.maxTime);
		}

		if (m_immortal && m_data.remainingTime < 1)
		{
			// チュートリアルなどでは1より小さくならない
			m_data.remainingTime = 1;
		}

		updatePos();

		drawUi(m_misrepresentedAsIfZero ? 0 : m_data.remainingTime);
	}

	void GiveDelta(ActorView self, double time, const Optional<bool>& isEnemyDamage)
	{
		m_outsideDelta += time;
		m_shadowOutsideDelta = m_outsideDelta;
		m_outsider.Kill();
		m_outsider = AnimateEasing<EaseInQuint>(self, &m_outsideDelta, 0.0, 1.0);
		if (isEnemyDamage.has_value()) m_fromEnemyDamage = isEnemyDamage.value();
	}

	void ExtendMax(ActorView self, double time)
	{
		m_data.maxTime += time;
		m_outsideDelta = -m_data.maxTime;
		m_shadowOutsideDelta = m_outsideDelta;
		m_outsider.Kill();
		m_outsider = AnimateEasing<EaseInQuint>(self, &m_outsideDelta, 0.0, 1.0);
	}

private:
	void updatePos()
	{
		if (not m_immortal && (m_data.remainingTime - m_outsideDelta <= 0))
		{
			m_targetPos = Scene::Center().movedBy(0, Scene::Center().y / 2);
		}
		else
		{
			m_targetPos = getDefaultPos();
		}

		if ((m_pos - m_targetPos).manhattanLength() < 1) return;

		m_pos = Math::Lerp(m_pos, m_targetPos, getToml<double>(U"pos_speed") * Scene::DeltaTime());
	}

	void drawUi(double remainingTime) const
	{
		const auto center = m_pos.asPoint();
		const auto mainColor = getToml<Color>(U"main_color");
		const auto emptyColor = getToml<Color>(U"empty_color");
		const auto damageColor = m_fromEnemyDamage
			                         ? getToml<Color>(U"enemy_damage_color")
			                         : getToml<Color>(U"penalty_damage_color");
		const auto healColor = getToml<Color>(U"heal_color");
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

		const double arcRate = Math::Clamp((remainingTime - m_outsideDelta) / arcAreaSeconds, 0.0, 1.0);
		const double arcAmount = 270_deg * arcRate;

		// 空部分
		(void)circle.drawArc(0_deg, 270_deg, -emptyThickness, circleOuter + emptyThickness, emptyColor);
		if (m_outsideDelta > 0)
		{
			// ダメージ部分
			const double damageArcAmount = 270_deg * Math::Clamp(remainingTime / arcAreaSeconds, 0.0, 1.0);
			(void)circle.drawArc(270_deg - damageArcAmount, damageArcAmount, 0, circleOuter, damageColor);
		}
		// 残量
		(void)circle.drawArc(270_deg - arcAmount, arcAmount, 0, circleOuter, mainColor);
		if (m_outsideDelta < 0 && remainingTime <= arcAreaSeconds)
		{
			// 回復部分
			const double healArcAmount = 270_deg * Math::Clamp(
				std::min(-m_outsideDelta, arcAreaSeconds - remainingTime) / arcAreaSeconds, 0.0, 1.0);
			(void)circle.drawArc(270_deg - arcAmount, healArcAmount, 0, circleOuter, healColor);
		}

		// アイコン描画
		(void)TextureAsset(U"⏳").resized(getToml<int>(U"icon_size")).drawAt(center);

		FontAsset(AssetKeys::RocknRoll_24_Bitmap)(U"ui_remaining_time"_localizef(static_cast<int>(remainingTime)))
			.draw(Arg::rightCenter = center.movedBy(getToml<Vec2>(U"text_offset")));

		// 長方形部分描画
		const auto rectTr = center.movedBy(0, -circleRadius - circleOuter);
		const auto rectH = circleOuter;
		const double rectUnit = getToml<double>(U"rect_unit_width");
		const double rectMaxRate = (m_data.maxTime - arcAreaSeconds) / arcAreaSeconds;
		const double rectRate =
			(std::min(m_data.maxTime, remainingTime - m_outsideDelta) - arcAreaSeconds) / arcAreaSeconds;
		const auto rectMaxW = rectUnit * rectMaxRate;
		const auto rectW = rectUnit * rectRate;
		if (rectMaxRate > 0)
			// 空部分
			(void)Rect(rectTr.movedBy(-rectMaxW, -emptyThickness), SizeF{rectMaxW, rectH}.asPoint())
				.draw(emptyColor);
		if (m_outsideDelta > 0 && remainingTime > arcAreaSeconds)
		{
			// ダメージ部分X
			const auto damageW = rectUnit * (remainingTime - arcAreaSeconds) / arcAreaSeconds;
			(void)Rect(rectTr.movedBy(-damageW, 0), SizeF{damageW, rectH}.asPoint())
				.draw(damageColor);
		}
		if (rectRate > 0)
			// 残量
			(void)Rect(rectTr.movedBy(-rectW, 0), SizeF{rectW, rectH}.asPoint())
				.draw(mainColor);
		if (rectRate > 0 && m_outsideDelta < 0)
		{
			// 回復部分
			const auto healW =
				rectUnit * std::min(-m_outsideDelta, m_data.maxTime - remainingTime) / arcAreaSeconds;
			(void)Rect(rectTr.movedBy(-rectW, 0), SizeF{std::min(healW, rectW), rectH}.asPoint())
				.draw(healColor);
		}
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
		p_impl->m_targetPos = getDefaultPos();
		p_impl->m_pos = p_impl->m_targetPos;
	}

	void UiTimeLimiter::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	void UiTimeLimiter::Damage(double time, bool isEnemyDamage)
	{
		p_impl->GiveDelta(*this, time, isEnemyDamage);
	}

	void UiTimeLimiter::Heal(double time)
	{
		p_impl->GiveDelta(*this, -time, none);
	}

	void UiTimeLimiter::ExtendMax(double time)
	{
		p_impl->ExtendMax(*this, time);
	}

	void UiTimeLimiter::ForceTerminate()
	{
		p_impl->m_data.remainingTime = 0;
	}

	void UiTimeLimiter::SetCountEnabled(bool enabled)
	{
		p_impl->m_countEnabled = enabled;
	}

	bool UiTimeLimiter::IsCountEnabled() const
	{
		return p_impl->m_countEnabled;
	}

	void UiTimeLimiter::SetImmortal(bool immortal)
	{
		p_impl->m_immortal = immortal;
	}

	void UiTimeLimiter::MisrepresentedAsIfZero()
	{
		p_impl->m_misrepresentedAsIfZero = true;
	}

	const TimeLimiterData& UiTimeLimiter::GetData() const
	{
		return p_impl->m_data;
	}
}
