#include "stdafx.h"
#include "UiFloorTransition.h"

#include "AssetKeys.h"
#include "Constants.h"
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
	bool m_isDraw = true;
	double m_radialRadius{};

	void Init()
	{
		m_renderTexture = RenderTexture(Scene::Size());
		m_maskTexture = RenderTexture(Scene::Size());
	}

	void Update()
	{
		(void)m_renderTexture.clear(Constants::HardDarkblue);

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

	void CloseRadial(ActorView self)
	{
		m_radialRadius = (Scene::Size() / 2).length();
		AnimateEasing<EaseInQuint>(self, &m_radialRadius, 0.0, getToml<double>(U"radial_duration"));
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

	void UiFloorTransition::CloseRadial()
	{
		p_impl->CloseRadial(*this);
	}
}
