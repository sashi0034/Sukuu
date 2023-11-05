#include "stdafx.h"
#include "PlayingPause.h"

#include "AssetKeys.h"

namespace Play
{
	void PlayingPause::Update()
	{
		if (not m_pauseAllowed) return;
		if (KeyEscape.down()) m_paused = not m_paused;
		if (not m_paused) return;

		const ScopedRenderStates2D sampler{SamplerState::ClampLinear};
		const Transformer2D transform{Mat3x2::Identity(), Transformer2D::Target::SetLocal};

		Rect(Scene::Size()).draw(ColorF{0.2, 0.7});

		FontAsset(AssetKeys::RocknRoll_Sdf_Bold)(U"ポーズ中")
			.drawAt(TextStyle::Outline(0.3, ColorF(0.2)), 96, Scene::Center(), Palette::White);
	}
}
