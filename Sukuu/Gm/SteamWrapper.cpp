#include "stdafx.h"
#include "SteamWrapper.h"
#include "steam_api.h"
#include <isteamgameserverstats.h>

#include "Util/Asserts.h"
#include "Util/ErrorLogger.h"
#include "Util/TomlDebugValueWrapper.h"

// ---------------------------------------------------------
// ブラウザで開いてデバッグコンソールを起動
// steam://open/console
//
// 最大のログレベルを設定
// set_spew_level 4 4
// ---------------------------------------------------------

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
			if (initResult != k_ESteamAPIInitResult_OK)
			{
				Util::ErrorLog(U"[Steam] Failed to initialize: {}\n{}"_fmt(
					static_cast<int>(initResult),
					Unicode::Widen(error)));

				return false;
			}

			if (not SteamUserStats()->RequestCurrentStats())
			{
				Util::ErrorLog(U"[Steam] Failed to request for current stats");
			}

#if _DEBUG
			if (Util::GetTomlDebugValueOf<bool>(U"steam_stat_reset"))
			{
				SteamUserStats()->ResetAllStats(true);
				SteamUserStats()->StoreStats();
			}
#endif
			return true;
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

	void CheckStoreSteamStatOfReach(bool isUra, int floor)
	{
		if (not Util::AssertStrongly(0 <= floor && floor <= 50)) return;

		const auto userStat = SteamUserStats();
		if (not userStat) return;

		std::string_view key = isUra ? "reached_ura" : "reached_standard";
		int prev;
		if (not userStat->GetStat(key.data(), &prev)) return;

		if (prev > floor) return;

		userStat->SetStat(key.data(), floor);
		userStat->StoreStats();

		if ((floor % 10) == 0)
		{
			// 実績解除
			const std::string achievement =
				isUra
					? fmt::format("URA_{:02}", floor)
					: fmt::format("STANDARD_{:02}", floor);
			if (not userStat->SetAchievement(achievement.c_str()))
			{
				Util::ErrorLog(U"[Steam] Failed to set achievement: {}"_fmt(Unicode::Widen(achievement)));
			}
		}

		userStat->StoreStats();
	}
}
