#pragma once
#include "GamepadButton.h"
#include "Util/ActorBase.h"

namespace Gm
{
	bool IsUsingGamepad();

	Input GetGamepadInput(GamepadButton button);
	bool IsGamepadPressed(GamepadButton button);
	bool IsGamepadUp(GamepadButton button);
	bool IsGamepadDown(GamepadButton button);

	Vec2 GetGamepadAxeL();

	class GamepadObserver : public Util::ActorBase
	{
	public:
		GamepadObserver();
		void Update() override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
