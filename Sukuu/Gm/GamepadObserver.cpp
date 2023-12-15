#include "stdafx.h"
#include "GamepadObserver.h"

#include "GameConfig.h"
#include "DialogGamepadRegister.h"

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
			// ゲームパッドのボタンを押したら認識する
			isUsingGamepad = (gamepad.buttons.contains_if([](auto&& b) { return b.down(); }));
		}
		else
		{
			const bool keyboardMouse = (Keyboard::GetAllInputs().contains_if([](auto&& i) { return i.down(); }))
				|| Mouse::GetAllInputs().contains_if([](auto&& i) { return i.down(); });
			isUsingGamepad = not keyboardMouse;
		}

		const bool recognizedNew = isUsingGamepad && currentGamepad != gamepad.getInfo().name;
		if (not recognizedNew) return;

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

namespace Gm
{
	struct GamepadObserver::Impl : ImplState
	{
	};

	bool IsUsingGamepad()
	{
		return s_instance->isUsingGamepad;
	}

	Input GetGamepadInput(GamepadButton button)
	{
		if (const auto gamepad = Gamepad(GamepadPlayer_0))
			return gamepad.buttons[s_instance->currentMap[button]];
		return {};
	}

	bool IsGamepadPressed(GamepadButton button)
	{
		if (const auto gamepad = Gamepad(GamepadPlayer_0))
			return gamepad.buttons[s_instance->currentMap[button]].pressed();
		return false;
	}

	bool IsGamepadUp(GamepadButton button)
	{
		if (const auto gamepad = Gamepad(GamepadPlayer_0))
			return gamepad.buttons[s_instance->currentMap[button]].up();
		return false;
	}

	bool IsGamepadDown(GamepadButton button)
	{
		if (const auto gamepad = Gamepad(GamepadPlayer_0))
			return gamepad.buttons[s_instance->currentMap[button]].down();
		return false;
	}

	Vec2 GetGamepadAxeL()
	{
		if (const auto gamepad = Gamepad(GamepadPlayer_0))
			return Vec2{gamepad.axes[0], gamepad.axes[1]};
		return {};
	}

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
}
