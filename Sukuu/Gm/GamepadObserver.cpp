#include "stdafx.h"
#include "GamepadObserver.h"

#include "GameConfig.h"
#include "DialogGamepadRegister.h"
#include "Util/Asserts.h"

namespace
{
	using namespace Gm;

	constexpr StringView literal_NotUsed = U"Not used";

	struct ImplState
	{
		bool isUsingGamepad{};
		String currentGamepad{literal_NotUsed};
		GamepadButtonMapping currentMap{};

		void Refresh();
	};

	void ImplState::Refresh()
	{
		const auto gamepad = Gamepad(GamepadPlayer_0);
		if (not gamepad)
		{
			// ゲームパッドを接続していない
			isUsingGamepad = false;
			currentGamepad = literal_NotUsed;
			return;
		}
		GameConfig::Instance();
		if (not isUsingGamepad)
		{
			// ゲームパッドのボタンを押したら isUsingGamepad: true
			isUsingGamepad = (gamepad.buttons.contains_if([](auto&& b) { return b.down(); }));
		}
		else
		{
			// キーボード、マウスのボタンを押したら isUsingGamepad: false
			const bool keyboardMouse = (Keyboard::GetAllInputs().contains_if([](auto&& i) { return i.down(); }))
				|| Mouse::GetAllInputs().contains_if([](auto&& i) { return i.down(); });
			isUsingGamepad = not keyboardMouse;
		}

		const bool recognizedNew = isUsingGamepad && currentGamepad != gamepad.getInfo().name;
		if (not recognizedNew) return;

		// XInput が接続されているなら、XInput レイアウトを使用
		if (XInput(GamepadPlayer_0).isConnected())
		{
			return;
		}

		// ゲームパッド名が違ったら、設定ファイルから参照
		for (auto&& map : GameConfig::Instance().gamepad_mapping)
		{
			if (map.first != gamepad.getInfo().name) continue;
			currentMap = map.second;
			return;
		}

		// キーマップを新規設定
		isUsingGamepad = false;
		if (const auto newMap = DialogGamepadRegister())
		{
			// 更新
			isUsingGamepad = true;
			currentGamepad = gamepad.getInfo().name;
			currentMap = newMap.value();
			GameConfig::Instance().gamepad_mapping[currentGamepad] = currentMap;
			GameConfig::Instance().RequestWrite();
		}
	}

	ImplState* s_instance;
}

// XInput Handling
// FIXME: Refactor
namespace
{
	RetrievedInput s_xInputRightTrigger{};
	RetrievedInput s_xInputLeftTrigger{};

	constexpr int XINPUT_GAMEPAD_TRIGGER_THRESHOLD = 30; // from "Xinput.h"

	class XInputWatcherAddon : public IAddon
	{
	public:
		bool m_previousRightTriggered{};
		bool m_previousLeftTriggered{};

		bool update() override
		{
			constexpr double threshold = static_cast<double>(XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / 255.0;

			if (not XInput(GamepadPlayer_0).isConnected())
			{
				// 未接続
				m_previousRightTriggered = false;
				m_previousLeftTriggered = false;
				s_xInputRightTrigger = {};
				s_xInputLeftTrigger = {};
				return true;
			}

			const bool rightTriggered = XInput(GamepadPlayer_0).rightTrigger >= threshold;
			const bool leftTriggered = XInput(GamepadPlayer_0).leftTrigger >= threshold;

			s_xInputRightTrigger = {
				.isDown = rightTriggered && not m_previousRightTriggered,
				.isPressed = rightTriggered,
				.isUp = not rightTriggered && m_previousRightTriggered
			};

			s_xInputLeftTrigger = {
				.isDown = leftTriggered && not m_previousLeftTriggered,
				.isPressed = leftTriggered,
				.isUp = not leftTriggered && m_previousLeftTriggered
			};

			m_previousLeftTriggered = leftTriggered;
			m_previousRightTriggered = rightTriggered;

			return true;
		}
	};

	RetrievedInput getXInputForButton(GamepadButton button)
	{
		const auto xInput = XInput(GamepadPlayer_0);
		switch (button)
		{
		case GamepadButton::A:
			return RetrievedInput::From(xInput.buttonA);
		case GamepadButton::B:
			return RetrievedInput::From(xInput.buttonB);
		case GamepadButton::X:
			return RetrievedInput::From(xInput.buttonX);
		case GamepadButton::Y:
			return RetrievedInput::From(xInput.buttonY);
		case GamepadButton::DRight:
			return RetrievedInput::From(xInput.buttonRight);
		case GamepadButton::DUp:
			return RetrievedInput::From(xInput.buttonUp);
		case GamepadButton::DLeft:
			return RetrievedInput::From(xInput.buttonLeft);
		case GamepadButton::DDown:
			return RetrievedInput::From(xInput.buttonDown);
		case GamepadButton::LB:
			return RetrievedInput::From(xInput.buttonLB);
		case GamepadButton::RB:
			return RetrievedInput::From(xInput.buttonRB);
		case GamepadButton::LT:
			return s_xInputLeftTrigger;
		case GamepadButton::RT:
			return s_xInputRightTrigger;
		case GamepadButton::Menu:
			return RetrievedInput::From(xInput.buttonMenu);
		case GamepadButton::Max:
			break;
		}
		return {};
	}
}

namespace Gm
{
	struct GamepadObserver::Impl : ImplState
	{
	};

	bool IsUsingGamepad()
	{
		return s_instance->isUsingGamepad;
	}

	RetrievedInput RetrievedInput::From(const Input& input)
	{
		return {.isDown = input.down(), .isPressed = input.pressed(), .isUp = input.up()};
	}

	RetrievedInput GetGamepadInput(GamepadButton button)
	{
		if (not Util::AssertStrongly(s_instance != nullptr)) return {};

		if (XInput(GamepadPlayer_0).isConnected())
			return getXInputForButton(button);

		if (const auto gamepad = Gamepad(GamepadPlayer_0))
			return RetrievedInput::From(gamepad.buttons[s_instance->currentMap[button]]);
		return {};
	}

	bool IsGamepadPressed(GamepadButton button)
	{
		if (not Util::AssertStrongly(s_instance != nullptr)) return {};

		if (XInput(GamepadPlayer_0).isConnected())
			return getXInputForButton(button).pressed();

		if (const auto gamepad = Gamepad(GamepadPlayer_0))
			return gamepad.buttons[s_instance->currentMap[button]].pressed();
		return false;
	}

	bool IsGamepadUp(GamepadButton button)
	{
		if (not Util::AssertStrongly(s_instance != nullptr)) return {};

		if (XInput(GamepadPlayer_0).isConnected())
			return getXInputForButton(button).up();

		if (const auto gamepad = Gamepad(GamepadPlayer_0))
			return gamepad.buttons[s_instance->currentMap[button]].up();
		return false;
	}

	bool IsGamepadDown(GamepadButton button)
	{
		if (not Util::AssertStrongly(s_instance != nullptr)) return {};

		if (XInput(GamepadPlayer_0).isConnected())
			return getXInputForButton(button).down();

		if (const auto gamepad = Gamepad(GamepadPlayer_0))
			return gamepad.buttons[s_instance->currentMap[button]].down();
		return false;
	}

	// Vec2 GetGamepadAxeL()
	// {
	// 	if (const auto gamepad = Gamepad(GamepadPlayer_0))
	// 		return Vec2{gamepad.axes[0], gamepad.axes[1]};
	// 	return {};
	// }

	void RefreshGamepad()
	{
		s_instance->Refresh();
	}

	GamepadObserver::GamepadObserver() :
		p_impl(std::make_shared<Impl>())
	{
		s_instance = p_impl.get();
	}

	void GamepadObserver::Update()
	{
		ActorBase::Update();
		p_impl->Refresh();
	}

	void InitXInputWatcherAddon()
	{
		Addon::Register<XInputWatcherAddon>(U"XInputWatcherAddon");
	}
}
