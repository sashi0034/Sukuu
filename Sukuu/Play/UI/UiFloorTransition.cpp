#include "stdafx.h"
#include "UiFloorTransition.h"

#include "AssetKeys.h"
#include "Assets.generated.h"
#include "Constants.h"
#include "Gm/LocalizedTextDatabase.h"
#include "Play/Other/FloorLevelDesign.h"
#include "Util/CoroUtil.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.ui_floor_transition." + key);
	}

	void drawGlyphs(
		const Array<OutlineGlyph>& glyphs, Vec2 penPos, double lengthRate, double period, const ColorF& color)
	{
		const double t = Periodic::Sawtooth0_1(period);
		for (const auto& glyph : glyphs)
		{
			const Transformer2D transform{Mat3x2::Translate(penPos + glyph.getOffset())};

			for (const auto& ring : glyph.rings)
			{
				const double length = ring.calculateLength(CloseRing::Yes);
				LineString z1 = ring.extractLineString(t * length, length * lengthRate, CloseRing::Yes);
				const LineString z2 = ring.extractLineString((t + 0.5) * length, length * lengthRate, CloseRing::Yes);
				(void)z1.append(z2.reversed()).drawClosed(3, color);
			}

			penPos.x += glyph.xAdvance;
		}
	}
}

struct Play::UiFloorTransition::Impl
{
	bool m_initialized{};
	RenderTexture m_renderTexture{};
	RenderTexture m_maskTexture{};
	bool m_isMasking = true;
	double m_radialRadius{};
	double m_maxRadialRadius{};
	Array<OutlineGlyph> m_glyphs{};
	double m_glyphWidth{};
	double m_glyphLength{};
	double m_glyphPeriod{};
	double m_textHeightScale{};
	String m_subheading{};
	std::pair<double, double> m_centerLineRange{};
	bool m_vesselMark{};
	bool m_redEnabled;

	void Init()
	{
		m_initialized = true;
		m_renderTexture = RenderTexture(Scene::Size());
		m_maskTexture = RenderTexture(Scene::Size());
	}

	void Update()
	{
		if (m_radialRadius == m_maxRadialRadius) return;

		if (m_isMasking)
			drawMasked();
		else
			drawDirect();
	}

	ActorWeak PerformOpen(ActorView self, int floorIndex)
	{
		return StartCoro(self, [self, this, floorIndex](YieldExtended yield)
		{
			processPerformOpen(yield, self, floorIndex);
		});
	}

	ActorWeak PerformClose(ActorView self)
	{
		m_maxRadialRadius = (Scene::Size() / 2).length();
		m_radialRadius = m_maxRadialRadius;
		return AnimateEasing<EaseInCirc>(self, &m_radialRadius, 0.0, getToml<double>(U"radial_duration"));
	}

private:
	void processPerformOpen(YieldExtended& yield, ActorView self, int floorIndex)
	{
		m_vesselMark = IsFloorExistVessel(floorIndex);

		m_maxRadialRadius = (Scene::Size() / 2).length();
		m_isMasking = false;

		yield.WaitForTime(0.5);
		m_centerLineRange.first = Scene::Size().x;
		m_centerLineRange.second = Scene::Size().x;
		AnimateEasing<EaseOutCubic>(self, &m_centerLineRange.first, 0.0, getToml<double>(U"line_duration"));
		yield.WaitForTime(getToml<double>(U"glyph_start"));

		AudioAsset(AssetSes::floor_transition).playOneShot();

		m_textHeightScale = 0;
		AnimateEasing<EaseOutBack>(self, &m_textHeightScale, 1.0, 0.5);

		m_subheading = Constants::MaxFloorIndex == floorIndex
			               ? U"last_layer_name"_localize
			               : U"remaining_layers"_localizef(Constants::MaxFloorIndex - floorIndex + 1);

		m_glyphs = FontAsset(AssetKeys::RocknRoll_72_Bitmap)
			.renderOutlines(U"layer_name"_localizef(Gm::LocalizeOrdinals(floorIndex)));
		m_glyphWidth = 0;
		for (auto&& g : m_glyphs)
		{
			m_glyphWidth += g.xAdvance;
		}
		m_glyphLength = 0;
		m_glyphPeriod = 2.4;
		yield.WaitForExpire(
			AnimateEasing<EaseInOutCubic>(self, &m_glyphLength, 0.96, getToml<double>(U"glyph_appear")));
		AnimateEasing<EaseInOutSine>(self, &m_glyphPeriod, 4.0, 0.5);

		yield.WaitForTime(getToml<double>(U"idle_duration"));

		AnimateEasing<EaseInOutCubic>(self, &m_glyphLength, 0.0, getToml<double>(U"glyph_appear"));
		AnimateEasing<EaseInCubic>(self, &m_centerLineRange.second, 0.0, getToml<double>(U"line_duration"));
		yield.WaitForExpire(AnimateEasing<EaseInCirc>(self, &m_textHeightScale, 0.0, 1.0));

		// 以下は、別のコルーチンとして実行
		StartCoro(self, [this, self](YieldExtended yield1)
		{
			// ここで2フレーム待機しないと、マップつくってない状態でオープンしてしまう
			yield1(2);
			m_isMasking = true;
			m_radialRadius = 0;
			AnimateEasing<EaseOutQuint>(
				self, &m_radialRadius, m_maxRadialRadius, getToml<double>(U"radial_duration"));
		});
	}

	void drawDirect() const
	{
		if (const auto target = Graphics2D::GetRenderTarget())
			(void)target.value().clear(Constants::HardDarkblue);
		else
			(void)Rect(Scene::Size()).draw(Constants::HardDarkblue);

		const auto glyphColor = m_redEnabled
			                        ? ColorF(U"#e84b3b")
			                        : ColorF(1);
		Line(m_centerLineRange.first, Scene::Center().y, m_centerLineRange.second, Scene::Center().y)
			.draw(getToml<double>(U"line_thickness"), glyphColor);

		[this]
		{
			const Transformer2D transform{Mat3x2::Scale(Vec2{1, m_textHeightScale} * 3.0, Scene::Center())};
			const auto glyphPos =
				Scene::Center().movedBy(Vec2{-m_glyphWidth, -72.0} / 2).movedBy(0, getToml<double>(U"glyph_y"));
			drawGlyphs(m_glyphs, glyphPos, m_glyphLength, m_glyphPeriod, Palette::White);

			if (m_vesselMark)
			{
				TextureAsset(U"💛").resized(48).drawAt(Scene::Center());
			}
		}();

		[this]
		{
			const Transformer2D transform{Mat3x2::Scale(Vec2{1, m_textHeightScale} * 1.0, Scene::Center())};
			auto&& text = FontAsset(AssetKeys::RocknRoll_72_Bitmap)(m_subheading);
			const auto pos =
				Scene::Center().movedBy(0, getToml<double>(U"subtitle_y"));
			(void)text.drawAt(pos, ColorF(getToml<Color>(U"subtitle_color")));
		}();
	}

	void drawMasked()
	{
		[this]()
		{
			ScopedRenderTarget2D target{m_renderTexture};
			drawDirect();
		}();

		[this]
		{
			// マスク描画
			ScopedRenderTarget2D target{m_maskTexture.clear(Palette::White)};
			(void)Circle(Scene::Center(), m_radialRadius).draw(Palette::Black);
		}();

		[this]
		{
			Graphics2D::SetPSTexture(1, m_maskTexture);
			const ScopedCustomShader2D shader{PixelShaderAsset(AssetKeys::PsMultiTextureMask)};
			m_renderTexture.drawAt(Scene::Center());
		}();
	}
};

namespace Play
{
	UiFloorTransition::UiFloorTransition() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void UiFloorTransition::Init()
	{
		p_impl->Init();
	}

	bool UiFloorTransition::IsInitialized() const
	{
		return p_impl->m_initialized;
	}

	void UiFloorTransition::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	double UiFloorTransition::OrderPriority() const
	{
		return 10000.0;
	}

	void UiFloorTransition::SetRed(bool enabled)
	{
		p_impl->m_redEnabled = enabled;
	}

	ActorWeak UiFloorTransition::PerformOpen(int floorIndex)
	{
		return p_impl->PerformOpen(*this, floorIndex);
	}

	ActorWeak UiFloorTransition::PerformClose()
	{
		return p_impl->PerformClose(*this);
	}
}
