#include "stdafx.h"
#include "PlayingPause.h"

#include "AssetKeys.h"
#include "Gm/DialogSettingConfigure.h"
#include "Gm/DialogYesNo.h"
#include "Gm/GamepadObserver.h"
#include "Play/PlayCore.h"
#include "Util/Utilities.h"

namespace Play
{
	PlayingPause::PlayingPause()
	{
		m_buttons.push_back(CornerButton(U"閉じる"_sv, [this]()
		{
			m_paused = false;
		}));

		m_buttons.push_back(CornerButton(U"設定"_sv, []()
		{
			Gm::DialogSettingConfigure();
		}));

		m_buttons.push_back(CornerButton(U"リタイア"_sv, [this]()
		{
			if (Gm::DialogYesNo(U"本当に諦めますか") != MessageBoxResult::Yes) return;
			PlayCore::Instance().GetTimeLimiter().ForceTerminate();
			m_paused = false;
		}));
	}

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

		if (not m_paused) return;

		const ScopedRenderStates2D sampler{SamplerState::ClampLinear};

		Rect(Scene::Size()).draw(ColorF{ColorF(Palette::Darkslateblue) * 0.7, 0.5});

		FontAsset(AssetKeys::RocknRoll_Sdf_Bold)(U"ポーズ中")
			.drawAt(TextStyle::Outline(0.3, ColorF(0.2)), 96, Scene::Center(), Palette::White);

		bool buttonHovered{};
		for (int i = 0; i < m_buttons.size(); ++i)
		{
			// ボタン描画
			buttonHovered |= m_buttons[i].Update(i, m_cursorIndex);
		}
		// if (not Gm::IsUsingGamepad() && not buttonHovered && IsSceneLeftClicked()) m_paused = false;

		m_cursorIndex = Util::Mod2<int>(
			m_cursorIndex + IsGamepadDown(Gm::GamepadButton::DUp) - IsGamepadDown(Gm::GamepadButton::DDown),
			m_buttons.size());
	}
}
