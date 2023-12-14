#include "stdafx.h"
#include "PlayingPause.h"

#include "AssetKeys.h"
#include "Gm/GamepadObserver.h"
#include "Util/Utilities.h"

namespace Play
{
	void PlayingPause::Update()
	{
		if (not m_pauseAllowed) return;

		const Transformer2D transform{Mat3x2::Identity(), TransformCursor::Yes, Transformer2D::Target::SetLocal};

#if not _DEBUG
		if (not Window::GetState().focused) m_paused = true;
#endif

		const bool requestDown = Gm::IsUsingGamepad()
			                         ? IsGamepadDown(Gm::GamepadButton::Menu)
			                         : KeyEscape.down();
		if (requestDown) m_paused = not m_paused;
		if (m_paused && Util::IsSceneLeftClicked()) m_paused = false;

		if (not m_paused) return;

		const ScopedRenderStates2D sampler{SamplerState::ClampLinear};

		Rect(Scene::Size()).draw(ColorF{0.2, 0.7});

		FontAsset(AssetKeys::RocknRoll_Sdf_Bold)(U"ポーズ中")
			.drawAt(TextStyle::Outline(0.3, ColorF(0.2)), 96, Scene::Center(), Palette::White);
	}
}
