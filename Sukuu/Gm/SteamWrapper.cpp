#include "stdafx.h"
#include "SteamWrapper.h"
#include "steam_api.h"

namespace
{
	struct SteamWrapperAddon : IAddon
	{
		bool init() override
		{
			if (SteamAPI_Init())
			{
				// FIXME: This is a temporary implementation.
				Console.writeln(U"SteamAPI_Init() succeeded.");
			}
			else
			{
				Console.writeln(U"SteamAPI_Init() failed.");
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
