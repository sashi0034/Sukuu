#include "stdafx.h"
#include "EndingHud.h"

#include "AssetKeys.h"
#include "Assets.generated.h"
#include "Constants.h"
#include "EndingOpenTransition.h"
#include "Gm/GameCursor.h"
#include "Gm/LocalizedTextDatabase.h"
#include "Play/PlayCore.h"
#include "Util/CoroUtil.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	using namespace Ending;

	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"ending.hud." + key);
	}

	struct SlideText
	{
		double x{};
		String text{};
	};
}

struct EndingHud::Impl
{
	Effect m_effect{};
	bool m_finished{};
	double m_initialMessageAlpha{};
	Array<SlideText> m_slideTexts{};
	double m_finalAlpha{};
	String m_finalInfo{};
	double m_closeAlpha{};
	double m_closeCloseAlpha{1.0};
	double m_staffsAlpha{};
	double m_sashiAlpha{};

	void Init(ActorView self, const Play::MeasuredSecondsArray& measured)
	{
		StartCoro(self, [this, self, &measured](YieldExtended yield)
		{
			flowchart(yield, self, measured);
		});
	}

	void Update()
	{
		const int marginY = getToml<int>(U"margin_y");
		const int availableY = Scene::Size().y - marginY * 2;

		const int textSize = getToml<int>(U"text_size");
		for (const auto i : step(m_slideTexts.size()))
		{
			auto&& text = m_slideTexts[i];
			if (text.text.isEmpty()) continue;
			auto&& font = FontAsset(AssetKeys::RocknRoll_Sdf);
			const auto pos = Vec2{text.x, marginY + (i + 0.5) * availableY / m_slideTexts.size()};
			font(text.text).regionAt(textSize, pos).stretched(Vec2{20, 4}).rounded(20).draw(ColorF(0.3, 0.7));
			font(text.text).drawAt(TextStyle::Outline(0.3, ColorF(0.4)),
			                       textSize,
			                       pos,
			                       Palette::White);
		}

		if (m_initialMessageAlpha > 0)
		{
			auto&& font = FontAsset(AssetKeys::RocknRoll_Sdf_Bold);
			font(U"achieved_last_50"_localize)
				.drawAt(TextStyle::Outline(0.3, ColorF(0.4, m_initialMessageAlpha)),
				        textSize * 2,
				        Scene::Center(),
				        ColorF(Palette::Azure, m_initialMessageAlpha));
		}

		if (m_closeAlpha > 0)
		{
			Rect(Scene::Size()).draw(ColorF(Constants::HardDarkblue, m_closeAlpha));
		}

		if (m_finalAlpha > 0)
		{
			const int finalY = getToml<int>(U"final_y");
			auto&& font = FontAsset(AssetKeys::RocknRoll_Sdf_Bold);
			const int finalSize = getToml<int>(U"final_size");
			font(U"completed_whole_dungeon"_localize)
				.drawAt(TextStyle::Outline(0.3, ColorF(0.4, m_closeCloseAlpha)),
				        finalSize,
				        Scene::Center().movedBy(0, -finalY),
				        ColorF(Palette::White, m_finalAlpha * m_closeCloseAlpha));
			font(m_finalInfo).drawAt(TextStyle::Outline(0.3, ColorF(0.4, m_closeCloseAlpha)),
			                         finalSize,
			                         Scene::Center().movedBy(0, finalY),
			                         ColorF(Palette::White, m_finalAlpha * m_closeCloseAlpha));
		}

		if (m_staffsAlpha > 0)
		{
			auto&& font = FontAsset(AssetKeys::RocknRoll_Sdf_Bold);
			font(U"Music Composer\n- bu\n\nProgram and Graphic Design\n- sashi")
				.drawAt(textSize, Scene::Center(), ColorF(1.0, m_staffsAlpha));
		}

		if (m_sashiAlpha > 0)
		{
			auto&& font = FontAsset(AssetKeys::RocknRoll_Sdf_Bold);
			font(U"Presented by sashi").drawAt(textSize, Scene::Center(), ColorF(1.0, m_sashiAlpha));
		}

		m_effect.update();
	}

private:
	void flowchart(YieldExtended& yield, ActorView self, const Play::MeasuredSecondsArray& measured)
	{
		const auto bgm = AudioAsset(AssetBgms::dear_my_rabbit);
		bgm.setLoop(true);
		bgm.play();

#if 0
#endif
		m_effect.add(CreateEndingOpenTransition({.basicDuration = 1.0, .fg = Constants::HardDarkblue}));
		yield.WaitForTime(3.0);

		yield.WaitForExpire(
			AnimateEasing<EaseOutSine>(self, &m_initialMessageAlpha, 1.0, 0.5));
		yield.WaitForTime(2.0);
		yield.WaitForExpire(
			AnimateEasing<EaseOutSine>(self, &m_initialMessageAlpha, 0.0, 0.5));

		constexpr int numLines = 10;
		for (int i = 0; i < numLines; ++i)
		{
			m_slideTexts.push_back(SlideText());
		}
		constexpr int slideSteps_5 = Constants::MaxFloorIndex / numLines;

		// 50層すべての記録を表示
		for (const auto d : step(slideSteps_5))
		{
			for (int i = 0; i < numLines; ++i)
			{
				const int floorIndex = i + 1 + d * 10;
				m_slideTexts[i].text = U"{}    - {} -"_fmt(
					U"layer_name"_localizef(Gm::LocalizeOrdinals(floorIndex)),
					FormatTimeSeconds(measured[floorIndex]));
				m_slideTexts[i].x = Scene::Center().x * 3;
				AnimateEasing<EaseOutCubic>(self, &m_slideTexts[i].x, static_cast<double>(Scene::Center().x), 0.5);
				yield.WaitForTime(0.15);
			}
			yield.WaitForTime(5.0);
			for (int i = 0; i < numLines; ++i)
			{
				AnimateEasing<EaseInCubic>(self, &m_slideTexts[i].x, static_cast<double>(-Scene::Center().x), 0.5);
				yield.WaitForTime(0.15);
			}
		}

		// スタッフ表示
		yield.WaitForExpire(AnimateEasing<EaseOutSine>(self, &m_staffsAlpha, 1.0, 0.5));
		yield.WaitForExpire(AnimateEasing<EaseOutSine>(self, &m_closeAlpha, 1.0, 2.0));
		yield.WaitForTime(3.0);
		yield.WaitForExpire(AnimateEasing<EaseOutSine>(self, &m_staffsAlpha, 0.0, 0.5));

		// 累計時間表示
		yield.WaitForTime(1.0);
		AnimateEasing<EaseOutCirc>(self, &m_finalAlpha, 1.0, 0.5);
		m_finalInfo = U"{} {}"_fmt(Gm::LocalizedText(U"total_cleared_time"), FormatTimeSeconds(measured.Sum()));

		yield.WaitForTime(2.0);

		yield.WaitForTrue([]() { return Gm::CheckConfirmSimply(); });

		bgm.stop(3.0s);
		yield.WaitForExpire(AnimateEasing<EaseOutSine>(self, &m_closeCloseAlpha, 0.0, 3.0));

		// Presented by sashi
		yield.WaitForExpire(AnimateEasing<EaseOutSine>(self, &m_sashiAlpha, 1.0, 1.0));
		yield.WaitForTime(3.0);
		yield.WaitForExpire(AnimateEasing<EaseOutSine>(self, &m_sashiAlpha, 0.0, 1.0));

		m_finished = true;
	}
};

namespace Ending
{
	EndingHud::EndingHud() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void EndingHud::Init(const Play::MeasuredSecondsArray& measured)
	{
		p_impl->Init(*this, measured);
	}

	void EndingHud::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	bool EndingHud::IsFinished() const
	{
		return p_impl->m_finished;
	}

	double EndingHud::OrderPriority() const
	{
		return 1000.0;
	}
}
