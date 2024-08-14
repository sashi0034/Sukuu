#pragma once
#include "GameConfig.h"

namespace Gm
{
	void InitSteamWrapperAddon();

	GameLanguage GetSteamLanguage();

	void CheckStoreSteamStatOfReach(bool isUra, int floor);
}
