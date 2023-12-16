#include "stdafx.h"
#include "CaveVision.h"

#include "AssetKeys.h"
#include "Assets.generated.h"
#include "Play/PlayCore.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	struct CaveVisionCb
	{
		float animRate = 0;
	};

	struct SoftShapeCb
	{
		float time = 0;
		float radius;
	};
}

struct Play::CaveVision::Impl
{
	ConstantBuffer<CaveVisionCb> m_caveVisionCb{};
	ConstantBuffer<SoftShapeCb> m_softShapeCb{};
	RenderTexture m_maskTexture{Scene::Size(), ColorF{1.0}};

	void UpdateMask(const Vec2& pos)
	{
		auto&& vision = PlayCore::Instance().GetPlayer().Vision();

		// 霧払い済み
		if (vision.mistRemoval)
		{
			m_maskTexture.clear(ColorF{1, 1});
			return;
		}

		// マスク描画
		ScopedRenderTarget2D target{m_maskTexture.clear(ColorF{0, 1})};

		Transformer2D t{
			Mat3x2::Translate(pos)
			.scaled({static_cast<double>(Scene::Size().x) / Scene::Size().y, 1.0}, pos)
		};
		// Transformer2D t{Mat3x2::Scale({static_cast<double>(Scene::Size().x) / Scene::Size().y, 1.0})};

		m_softShapeCb->time += GetDeltaTime() * vision.heartbeatRate;
		m_softShapeCb->radius = GetTomlParameter<float>(U"play.cave_vision.default_vision_radius") * vision.radiusRate;
		Graphics2D::SetVSConstantBuffer(1, m_softShapeCb);
		const ScopedCustomShader2D shader{VertexShaderAsset(AssetKeys::VsCaveVision)};

		Graphics2D::DrawTriangles(360);
	}

	void UpdateScreen()
	{
		// スクリーン描画
		constexpr float animSpeed = 0.3f;
		m_caveVisionCb->animRate += GetDeltaTime() * animSpeed;
		Graphics2D::SetPSConstantBuffer(1, m_caveVisionCb);
		const ScopedCustomShader2D shader{PixelShaderAsset(AssetKeys::PsCaveVision)};

		Graphics2D::SetPSTexture(1, TextureAsset(AssetImages::aqua_noise));
		Graphics2D::SetPSTexture(2, TextureAsset(AssetImages::cosmos_noise));
		Graphics2D::SetPSTexture(3, m_maskTexture);

		(void)m_maskTexture({0, 0}, Scene::Size()).draw(
			GetTomlParameter<Color>(U"play.cave_vision.dark_color"));
	}
};

namespace Play
{
	CaveVision::CaveVision() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void CaveVision::UpdateMask(const Vec2& pos)
	{
		p_impl->UpdateMask(pos);
	}

	void CaveVision::UpdateScreen()
	{
		p_impl->UpdateScreen();
	}
}
