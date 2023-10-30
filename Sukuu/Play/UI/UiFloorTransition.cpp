#include "stdafx.h"
#include "UiFloorTransition.h"

#include "AssetKeys.h"
#include "Constants.h"
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
}

struct Play::UiFloorTransition::Impl
{
	RenderTexture m_renderTexture{};
	RenderTexture m_maskTexture{};
	bool m_isMasking = true;
	double m_radialRadius{};
	String m_message{};

	void Init()
	{
		m_renderTexture = RenderTexture(Scene::Size());
		m_maskTexture = RenderTexture(Scene::Size());
	}

	void Update()
	{
		if (m_isMasking)
			drawMasked();
		else
			drawDirect();
	}

	void drawDirect()
	{
		if (const auto target = Graphics2D::GetRenderTarget())
			(void)target.value().clear(Constants::HardDarkblue);
		else
			(void)Rect(Scene::Size()).draw(Constants::HardDarkblue);

		FontAsset(AssetKeys::RocknRoll_Msdf)(m_message).drawAt(Scene::Center());
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

	ActorWeak PerformOpen(ActorView self, int floorIndex)
	{
		return StartCoro(self, [self, this, floorIndex](YieldExtended yield)
		{
			processPerformOpen(yield, self, floorIndex);
		});
	}

	ActorWeak PerformClose(ActorView self)
	{
		m_radialRadius = (Scene::Size() / 2).length();
		return AnimateEasing<EaseInQuint>(self, &m_radialRadius, 0.0, getToml<double>(U"radial_duration"));
	}

private:
	void processPerformOpen(YieldExtended& yield, ActorView self, int floorIndex)
	{
		m_isMasking = false;
		m_message = U"第 {} 層\n~ 最奥部まで あと {}~"_fmt(floorIndex, 50 - floorIndex);
		yield.WaitForTime(3.0);
		m_isMasking = true;
		m_radialRadius = 0;
		AnimateEasing<EaseInQuint>(
			self, &m_radialRadius, (Scene::Size() / 2).length(), getToml<double>(U"radial_duration"));
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

	void UiFloorTransition::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	double UiFloorTransition::OrderPriority() const
	{
		return 10000.0;
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
