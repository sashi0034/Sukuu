#include "stdafx.h"
#include "GamepadObserver.h"

#include "GameConfig.h"
#include "GamepadRegistering.h"

namespace
{
	using namespace Gm;

	constexpr StringView literal_NotUsed = U"Not used";

	struct ImplState
	{
		bool isUsingGamepad{};
		String currentGamepad{literal_NotUsed};
		GamepadButtonMapping currentMap{};
	};

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

	GamepadObserver::GamepadObserver() :
		p_impl(std::make_shared<Impl>())
	{
		s_instance = p_impl.get();
	}

	void GamepadObserver::Update()
	{
		ActorBase::Update();
		const auto gamepad = Gamepad(GamepadPlayer_0);
		if (not gamepad)
		{
			// ゲームパッドを接続していない
			p_impl->isUsingGamepad = false;
			p_impl->currentGamepad = literal_NotUsed;
			return;
		}
		GameConfig::Instance();
		if (not p_impl->isUsingGamepad)
		{
			// ゲームパッドのボタンを押したら認識する
			p_impl->isUsingGamepad = (gamepad.buttons.contains_if([](auto&& b) { return b.down(); }));
		}
		else
		{
			p_impl->isUsingGamepad = not(Keyboard::GetAllInputs().contains_if([](auto&& i) { return i.down(); }));
		}

		const bool recognizedNew = p_impl->isUsingGamepad && p_impl->currentGamepad != gamepad.getInfo().name;
		if (not recognizedNew) return;

		// ゲームパッド名が違ったら、設定ファイルから参照
		for (auto&& map : GameConfig::Instance().gamepad.mapping)
		{
			if (map.first != gamepad.getInfo().name) continue;
			p_impl->currentMap = map.second;
			return;
		}

		// キーマップを新規設定
		if (const auto newMap = DialogGamepadRegistering())
		{
			// 更新
			p_impl->currentGamepad = gamepad.getInfo().name;
			p_impl->currentMap = newMap.value();
			GameConfig::Instance().gamepad.mapping[p_impl->currentGamepad] = p_impl->currentMap;
			GameConfig::Instance().RequestWrite();
		}
		else
		{
			p_impl->isUsingGamepad = false;
		}
	}
}
