#include "stdafx.h"
#include "SteamWrapper.h"
#include "steam_api.h"
#include <isteamgameserverstats.h>

namespace
{
	constexpr int steamAppId = 3147480;

	struct SteamWrapperAddon : IAddon
	{
		bool init() override
		{
			if (SteamAPI_RestartAppIfNecessary(steamAppId))
			{
				Console.writeln(U"SteamAPI_RestartAppIfNecessary() true");
			}
			else
			{
				Console.writeln(U"SteamAPI_RestartAppIfNecessary() false");
			}

			SteamErrMsg err{};
			const auto initRes = SteamAPI_InitEx(&err);
			if (initRes == k_ESteamAPIInitResult_OK)
			{
				// FIXME: This is a temporary implementation.
				Console.writeln(U"SteamAPI_Init() succeeded.");
			}
			else
			{
				Console.writeln(U"SteamAPI_Init() failed. {}"_fmt(static_cast<int>(initRes)));
				Console.writeln(Unicode::Widen(err));
			}

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
}
