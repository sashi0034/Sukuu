#pragma once
#include "GameConfig.h"

namespace Gm
{
	[[nodiscard]]
	GameConfig DialogSettingConfigure(const GameConfig& current = GameConfig::Instance());
}
