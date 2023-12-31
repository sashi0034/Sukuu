﻿#include "stdafx.h"
#include "TutorialMessenger.h"

#include "AssetKeys.h"
#include "Util/CoroUtil.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"tutorial.messenger." + key);
	}
}

struct Tutorial::TutorialMessenger::Impl
{
	double m_time{};
	String m_message{};
	double m_alpha{};
	ActorWeak m_startedCoro{};

	void Update()
	{
		if (m_message.empty()) return;

		m_time += Scene::DeltaTime();
		const auto&& font = FontAsset(AssetKeys::RocknRoll_Sdf);
		const double fontSize = getToml<int>(U"font_size");
		const Vec2 textSize = font(m_message).region().scaled((fontSize / font.fontSize())).size;
		drawText(font,
		         fontSize,
		         m_message,
		         Rect{{0, 0}, Scene::Size()}.bottomCenter() - textSize / 2 + Vec2{0, -getToml<int>(U"bottom")},
		         ColorF{Palette::White, m_alpha},
		         m_time);
	}

	void StartMessage(ActorView self, const String& message, double duration)
	{
		m_message = message;
		m_time = 0;
		m_alpha = 1.0;
		if (duration < 0) return;

		m_startedCoro.Kill();
		m_startedCoro = StartCoro(self, [this, self, duration](YieldExtended yield)
		{
			const double fadeDuration = getToml<double>(U"fade_duration");
			yield.WaitForTime(duration - fadeDuration, Scene::DeltaTime);
			HideMessage(self, fadeDuration);
		});
	}

	void HideMessage(ActorView self, double fadeDuration = getToml<double>(U"fade_duration"))
	{
		AnimateEasing<EaseInQuad, EaseOption::Default | EaseOption::IgnoreTimeScale>(
			self, &m_alpha, 0.0, fadeDuration);
	}

private:
	// 参考: // https://zenn.dev/reputeless/books/siv3d-documentation/viewer/sample-visual
	static void drawText(
		const Font& font, double fontSize, const String& text, const Vec2& pos, const ColorF& color, double t)
	{
		const double scale = (fontSize / font.fontSize());
		Vec2 penPos = pos;
		const ScopedCustomShader2D shader{Font::GetPixelShader(font.method())};

		for (auto&& [i, glyph] : Indexed(font.getGlyphs(text)))
		{
			if (glyph.codePoint == U'\n')
			{
				penPos.x = pos.x;
				penPos.y += (font.height() * scale);
				continue;
			}

			constexpr double characterPerSec = 0.1;
			const double targetTime = (i * characterPerSec);

			if (t < targetTime)
			{
				break;
			}

			textEffect1(penPos, scale, glyph, color, (t - targetTime));

			penPos.x += (glyph.xAdvance * scale);
		}
	}

	// 文字が上からゆっくり降ってくる表現
	static void textEffect1(const Vec2& penPos, double scale, const Glyph& glyph, const ColorF& color, double t)
	{
		const double y = EaseInQuad(Saturate(1 - t / 0.3)) * -20.0;
		const double a = color.a * Min(t / 0.3, 1.0);
		glyph.texture.scaled(scale).draw(penPos + glyph.getOffset(scale) + Vec2{0, y}, ColorF{color, a});
	}
};

namespace Tutorial
{
	TutorialMessenger::TutorialMessenger() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void TutorialMessenger::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	double TutorialMessenger::OrderPriority() const
	{
		return 1000.0;
	}

	void TutorialMessenger::ShowMessage(const String& message, double duration)
	{
		p_impl->StartMessage(*this, message, duration);
	}

	void TutorialMessenger::ShowMessageForever(const String& message)
	{
		p_impl->StartMessage(*this, message, -1);
	}

	void TutorialMessenger::HideMessage()
	{
		p_impl->HideMessage(*this);
	}
}
