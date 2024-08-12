#include "stdafx.h"
#include "UiMessenger.h"

#include "AssetKeys.h"
#include "Util/CoroUtil.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.ui_messenger." + key);
	}
}

struct Play::UiMessenger::Impl
{
	double m_time{};
	String m_message{};
	double m_alpha{};
	ActorWeak m_startedCoro{};
	double m_characterPerSecond{0.05}; // TODO: ゲーム言語に応じて速度変更をする

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
	void drawText(
		const Font& font, double fontSize, const String& text, const Vec2& pos, const ColorF& color, double t) const
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

			const double characterPerSec = m_characterPerSecond;
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

namespace Play
{
	UiMessenger::UiMessenger() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void UiMessenger::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	double UiMessenger::OrderPriority() const
	{
		return 1000.0;
	}

	void UiMessenger::ShowMessage(const String& message, double duration)
	{
		p_impl->StartMessage(*this, message, duration);
	}

	void UiMessenger::ShowMessageForever(const String& message)
	{
		p_impl->StartMessage(*this, message, -1);
	}

	void UiMessenger::HideMessage()
	{
		p_impl->HideMessage(*this);
	}
}
