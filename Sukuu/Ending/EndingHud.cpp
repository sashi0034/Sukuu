#include "stdafx.h"
#include "EndingHud.h"

#include "AssetKeys.h"
#include "Constants.h"
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
	Array<SlideText> m_slideTexts{};
	double m_finalAlpha{};
	String m_finalInfo{};

	void Init(ActorView self)
	{
		StartCoro(self, [this, self](YieldExtended yield)
		{
			flowchart(yield, self);
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
			font(text.text).drawAt(TextStyle::Outline(0.3, ColorF(0.4)),
			                       textSize,
			                       Vec2{text.x, marginY + i * availableY / m_slideTexts.size()},
			                       Palette::White);
		}

		if (m_finalAlpha > 0)
		{
			const int finalY = getToml<int>(U"final_y");
			auto&& font = FontAsset(AssetKeys::RocknRoll_Sdf_Bold);
			const int finalSize = getToml<int>(U"final_size");
			font(U"よくぞここまでたどり着きましたね\nお疲れ様でした")
				.drawAt(TextStyle::Outline(0.3, ColorF(0.4)),
				        textSize,
				        Scene::Center().movedBy(0, -finalY),
				        ColorF(Palette::White, m_finalAlpha));
			font(m_finalInfo).drawAt(TextStyle::Outline(0.3, ColorF(0.4)),
			                         finalSize,
			                         Scene::Center().movedBy(0, finalY),
			                         ColorF(Palette::White, m_finalAlpha));
		}
	}

private:
	void flowchart(YieldExtended& yield, ActorView self)
	{
		constexpr int numLines = 10;
		for (int i = 0; i < numLines; ++i)
		{
			m_slideTexts.push_back(SlideText());
		}
		constexpr int slideSteps_5 = Constants::MaxFloorIndex / numLines;
		for (const auto d : step(slideSteps_5))
		{
			for (int i = 0; i < numLines; ++i)
			{
				m_slideTexts[i].text = U"第 {} 層    - 12:34.00 -"_fmt(i + 1);
				m_slideTexts[i].x = Scene::Center().x * 3;
				AnimateEasing<EaseOutCubic>(self, &m_slideTexts[i].x, static_cast<double>(Scene::Center().x), 0.5);
				yield.WaitForTime(0.15);
			}
			yield.WaitForTime(1.5);
			for (int i = 0; i < numLines; ++i)
			{
				AnimateEasing<EaseInCubic>(self, &m_slideTexts[i].x, static_cast<double>(-Scene::Center().x), 0.5);
				yield.WaitForTime(0.15);
			}
		}

		yield.WaitForTime(1.0);
		AnimateEasing<EaseOutCirc>(self, &m_finalAlpha, 1.0, 0.5);
		m_finalInfo = U"迷宮踏破記録 12:34.56";
	}
};

namespace Ending
{
	EndingHud::EndingHud() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void EndingHud::Init()
	{
		p_impl->Init(*this);
	}

	void EndingHud::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}
}
