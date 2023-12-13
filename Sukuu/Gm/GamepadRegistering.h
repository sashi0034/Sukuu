#pragma once

namespace Gm
{
	enum class GamepadButton
	{
		A,
		B,
		X,
		Y,
		DRight,
		DUp,
		DLeft,
		DDown,
		LB,
		RB,
		LT,
		RT,
		Menu,
		Max
	};

	constexpr int GamepadButtonSize = static_cast<int>(GamepadButton::Max);

	void DialogGamepadRegistering();
}
