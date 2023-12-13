#pragma once
#include "GamepadObserver.h"

namespace Gm
{
	[[nodiscard]]
	Optional<GamepadButtonMap<int>> DialogGamepadRegistering();
}
