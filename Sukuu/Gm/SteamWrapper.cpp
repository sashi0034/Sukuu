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

	// ---------------------------------------------------------
	// https://partner.steamgames.com/doc/features/overlay
	// OpenGL/D3Dデバイスを初期化する前に、必ずSteamAPI_Initを呼び出すようにしてください。呼び出しが無いと、この関数がデバイスの作成をフックできなくなります。
	// ---------------------------------------------------------
	// OpenSiv3D は Main の中に入る時点でデバイスを初期化しているため、上記の条件を満たさない
	// そのため、Hack 的になるがグローバルオブジェクトのコンストラクタを利用して Main の前に SteamAPI_InitEx を行う
	class SteamInitializer
	{
	public:
		String errorMessage{};
		bool initialized{};

		SteamInitializer()
		{
			if (SteamAPI_RestartAppIfNecessary(steamAppId))
			{
				errorMessage = U"[Steam] Restart required via Steam.";
				return;
			}

			SteamErrMsg error{};
			const auto initResult = SteamAPI_InitEx(&error);
			if (initResult != k_ESteamAPIInitResult_OK)
			{
				errorMessage = U"[Steam] Failed to initialize: {}\n{}"_fmt(
					static_cast<int>(initResult),
					Unicode::Widen(error));
				return;
			}

			initialized = true;
		}
	};

	SteamInitializer s_initializer{};

	bool s_overlayActivated = false;

	struct SteamWrapperAddon : IAddon
	{
		STEAM_CALLBACK(SteamWrapperAddon, onGameOverlayActivated, GameOverlayActivated_t);

		bool init() override
		{
			if (not s_initializer.initialized)
			{
#if _DEBUG
				if (Util::GetTomlDebugValueOf<bool>(U"steam_no_require")) return false;
#endif
				if (not s_initializer.errorMessage.empty()) Util::ErrorLog(s_initializer.errorMessage);
				System::Exit();
				return false;
			}

			if (const auto userStat = SteamUserStats())
			{
				if (not userStat->RequestCurrentStats())
				{
					Util::ErrorLog(U"[Steam] Failed to request for current stats");
				}
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

		bool update() override
		{
			if (s_initializer.initialized)
			{
				SteamAPI_RunCallbacks();
			}

			return true;
		}
	};

	// オーバーレイ表示時のコールバック
	void SteamWrapperAddon::onGameOverlayActivated(GameOverlayActivated_t* pCallback)
	{
		if (pCallback->m_nAppID != steamAppId) return;

		s_overlayActivated = pCallback->m_bActive;
	}
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

	void CheckStoreSteamStatOfCleared(bool isUra, int floor)
	{
		if (not Util::AssertStrongly(0 <= floor && floor <= 50)) return;

		const auto userStat = SteamUserStats();
		if (not userStat) return;

		// reached となっているが、本当は cleared のつもり (Steam の実績 Web UI をもう触りたくない...)
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

	void ShowSteamKeyconfig(int playerIndex)
	{
		const auto steamFriends = SteamFriends();
		const auto steamInput = SteamInput();
		if (not steamFriends || not steamInput) return;

		if (IsSteamOverlayActivated()) return;

		// const auto steamUtils = SteamUtils();
		// if (not steamUtils) return;
		// SteamUtils()->IsSteamInBigPictureMode() の判定の導入も検討したが、Big Picture でも ShowBindingPanel は微妙だった

		if (not Window::GetState().fullscreen)
		{
			steamInput->ShowBindingPanel(steamInput->GetControllerForGamepadIndex(playerIndex));
		}
		else
		{
			// フルスクリーンで ShowBindingPanel をするとゲームウィンドウの背面に表示されるため、代わりに Overlay 表示をする
			steamFriends->ActivateGameOverlay("Settings");
		}
	}

	bool IsSteamOverlayActivated()
	{
		return s_overlayActivated;
	}
}
