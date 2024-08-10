#include "stdafx.h"
#include "LoungeWaterSurface.h"

#include "Play/Forward.h"
#include "Play/Other/CaveVisionCb.h"
#include "Util/Utilities.h"

namespace
{
	using namespace Lounge;
}

struct LoungeWaterSurface::Impl
{
	ConstantBuffer<Play::CaveVisionCb> m_caveVisionCb{};

	void Render(const render_args& args)
	{
		Transformer2D t2d{Mat3x2::Identity(), Transformer2D::Target::SetLocal};

		m_caveVisionCb->masked = false;

		m_caveVisionCb->positionOffset = args.cameraPos;

		constexpr float animSpeed = 0.2f;
		m_caveVisionCb->animRate += Util::GetDeltaTime() * animSpeed;

		const auto shader = PrepareCaveVision(m_caveVisionCb);

		(void)Rect(Scene::Size()).draw(ColorF(0.1, 0.2, 0.5, 1));
	}
};

namespace Lounge
{
	LoungeWaterSurface::LoungeWaterSurface() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void LoungeWaterSurface::Render(const render_args& args)
	{
		p_impl->Render(args);
	}
}
