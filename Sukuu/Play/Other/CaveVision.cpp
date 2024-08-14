#include "stdafx.h"
#include "CaveVision.h"

#include "AssetKeys.h"
#include "CaveVisionCb.h"
#include "Play/PlayCore.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
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
	RenderTexture m_maskTexture{};
	bool m_enabled{true};

	void UpdateMask(const Vec2& pos)
	{
		if (not m_enabled) return;

		if (m_maskTexture.isEmpty())
		{
			m_maskTexture = RenderTexture{Scene::Size(), ColorF{1.0}};
		}

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
		const ScopedCustomShader2D shader{VertexShaderAsset(AssetKeys::VsSoftShape)};

		Graphics2D::DrawTriangles(360);
	}

	void UpdateScreen()
	{
		if (not m_enabled || m_maskTexture.isEmpty()) return;

		// スクリーン描画
		constexpr float animSpeed = 0.3f;
		m_caveVisionCb->animRate += GetDeltaTime() * animSpeed;

		const auto shader = PrepareCaveVision(m_caveVisionCb);

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

	void CaveVision::SetEnabled(bool enabled)
	{
		p_impl->m_enabled = enabled;
		if (not enabled)
		{
			p_impl->m_maskTexture = {};
		}
	}
}
