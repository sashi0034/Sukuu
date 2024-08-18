#pragma once
#include "GameConfig.h"

namespace Gm
{
	void InitSteamWrapperAddon();

	GameLanguage GetSteamLanguage();

	void CheckStoreSteamStatOfCleared(bool isUra, int floor);
}
