#pragma once
#include "GameConfig.h"

namespace Gm
{
	void InitSteamWrapperAddon();

	GameLanguage GetSteamLanguage();

	void CheckStoreSteamStatOfCleared(bool isUra, int floor);

	void ShowSteamKeyconfig(int playerIndex = 0);

	bool IsSteamOverlayActivated();
}
