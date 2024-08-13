#include "stdafx.h"
#include "SteamWrapper.h"
#include "steam_api.h"
#include <isteamgameserverstats.h>

#include "Util/ErrorLogger.h"

namespace
{
	constexpr int steamAppId = 3147480;

	struct SteamWrapperAddon : IAddon
	{
		bool init() override
		{
			// if (SteamAPI_RestartAppIfNecessary(steamAppId)) // TODO

			SteamErrMsg error{};
			const auto initResult = SteamAPI_InitEx(&error);
			if (initResult == k_ESteamAPIInitResult_OK) return true;

			Util::ErrorLog(U"Steam API Initialization failed: {}.\n{}"_fmt(
				static_cast<int>(initResult),
				Unicode::Widen(error)));

			return false;
		}

		~SteamWrapperAddon() override
		{
			SteamAPI_Shutdown();
		}
	};
}

namespace Gm
{
	void InitSteamWrapperAddon()
	{
		Addon::Register<SteamWrapperAddon>(U"SteamWrapperAddon");
	}

	// https://partner.steamgames.com/doc/sdk/api

	GameLanguage GetSteamLanguage()
	{
		const auto steamApps = SteamApps();
		if (not steamApps) return GameLanguage::Ja;

		// https://partner.steamgames.com/doc/api/ISteamApps?l=japanese#GetCurrentGameLanguage
		const std::string c = SteamApps()->GetCurrentGameLanguage();

		if (c == "japanese") return GameLanguage::Ja;
		if (c == "koreana") return GameLanguage::Ko;
		if (c == "schinese") return GameLanguage::Cs;
		if (c == "tchinese") return GameLanguage::Ct;

		return GameLanguage::En;
	}
}
