#include "stdafx.h"
#include "PlayingPause.h"

#include "AssetKeys.h"
#include "CornerButton.h"
#include "Gm/DialogSettingConfigure.h"
#include "Gm/DialogMessageBox.h"
#include "Gm/GamepadObserver.h"
#include "Play/PlayCore.h"
#include "Util/Utilities.h"
#include "Gm/LocalizedTextDatabase.h"

struct Play::PlayingPause::Impl
{
	bool m_paused{};
	bool m_pauseAllowed{true};

	Array<CornerButton> m_buttons{};
	int m_cursorIndex{};

	void Init(bool enableRetire)
	{
		m_buttons.push_back(CornerButton(U"pause_close"_sv, [this]()
		{
			m_paused = false;
		}));

		m_buttons.push_back(CornerButton(U"pause_setting"_sv, []()
		{
			Gm::DialogSettingConfigure();
		}));

		if (enableRetire)
		{
			m_buttons.push_back(CornerButton(U"pause_retire"_sv, [this]()
			{
				if (Gm::DialogYesNo(U"pause_confirm_retire"_localize) != MessageBoxResult::Yes) return;
				PlayCore::Instance().GetTimeLimiter().ForceTerminate();
				m_paused = false;
			}));
		}
	}

	void Update()
	{
		if (not m_pauseAllowed) return;
		if (m_buttons.size() == 0) return;

		const Transformer2D transform{Mat3x2::Identity(), TransformCursor::Yes, Transformer2D::Target::SetLocal};

#if not _DEBUG
		if (not Window::GetState().focused) m_paused = true;
#endif

		const bool requestDown = Gm::IsUsingGamepad()
			                         ? IsGamepadDown(Gm::GamepadButton::Menu)
			                         : (KeyEscape.down() || KeyTab.down());
		if (requestDown) m_paused = not m_paused;

		if (not m_paused) return;

		const ScopedRenderStates2D sampler{SamplerState::ClampLinear};

		Rect(Scene::Size()).draw(ColorF{ColorF(Palette::Darkslateblue) * 0.7, 0.5});

		FontAsset(AssetKeys::RocknRoll_Sdf_Bold)(U"pause_title"_localize)
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
};

namespace Play
{
	PlayingPause::PlayingPause() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void PlayingPause::Init(bool enableRetire)
	{
		p_impl->Init(enableRetire);
	}

	bool PlayingPause::IsPaused() const
	{
		return p_impl->m_pauseAllowed && p_impl->m_paused;
	}

	void PlayingPause::SetAllowed(bool e)
	{
		p_impl->m_pauseAllowed = e;
	}

	void PlayingPause::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	void PlayingPause::AddButtonCancelTutorial(const std::function<void()>& callback)
	{
		p_impl->m_buttons.push_back(CornerButton(U"pause_cancel_tutorial"_sv, [this, callback]()
		{
			if (Gm::DialogYesNo(U"pause_confirm_cancel_tutorial"_localize) != MessageBoxResult::Yes) return;
			callback();
		}));
	}

	void PlayingPause::AddButtonExitGame()
	{
		p_impl->m_buttons.push_back(CornerButton(U"pause_exit_game"_sv, [this]()
		{
			if (Gm::DialogYesNo(U"pause_confirm_exit_game"_localize) != MessageBoxResult::Yes) return;
			System::Exit();
		}));
	}
}
