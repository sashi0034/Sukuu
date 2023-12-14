#pragma once
#include "GamepadButton.h"
#include "Util/ActorBase.h"

namespace Gm
{
	bool IsUsingGamepad();

	bool IsGamepadPressed(GamepadButton button);
	bool IsGamepadUp(GamepadButton button);
	bool IsGamepadDown(GamepadButton button);

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
