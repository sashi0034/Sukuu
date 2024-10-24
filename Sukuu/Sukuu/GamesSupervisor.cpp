#include "stdafx.h"
#include "GamesSupervisor.h"

#include "Constants.h"
#include "Gm/GameSavedata.h"
#include "Ending/EndingScene.h"
#include "Gm\DialogSettingConfigure.h"
#include "Gm/DialogGamepadRegister.h"
#include "Gm/DialogMessageBox.h"
#include "Gm/LocalizedTextDatabase.h"
#include "Gm/SteamWrapper.h"
#include "Lounge/LoungeScene.h"
#include "Play/PlayingUra.h"
#include "Play/PlayScene.h"
#include "Play/Other/FloorLevelDesign.h"
#include "Play/Other/FloorBgmData.h"
#include "Title/TitleScene.h"
#include "Tutorial/TutorialScene.h"
#include "Util/CoroUtil.h"
#include "Util/TomlDebugValueWrapper.h"

namespace
{
	using namespace Util;
	using namespace Gm;

#if _DEBUG
	template <typename T>
	inline T debugToml(const String& key)
	{
		return GetTomlDebugValueOf<T>(key);
	}
#endif

	constexpr double initialTimeLimit = 90.0;

	ReachedRecord& getReachedRecord(GameSavedata& data)
	{
		return data.GetReached(Play::IsPlayingUra());
	}

	TemporaryRecord& getTemporaryRecord(GameSavedata& data)
	{
		return data.GetTemporary(Play::IsPlayingUra());
	}
}

struct Sukuu::GamesSupervisor::Impl
{
	void FlowchartAsync(YieldExtended& yield, ActorView self)
	{
		initPlayData();

		while (true)
		{
			flowchartLoop(yield, self);
		}
	}

private:
	GameSavedata m_savedata{};
	Play::PlaySingletonData m_playData{};

	void flowchartLoop(YieldExtended& yield, ActorView self)
	{
		bool triedTutorial{};
		bool fromTitle{};

		yield();
#if _DEBUG
		Play::SetPlayingUra(debugToml<bool>(U"ura_enable"));

		const auto entryPoint = debugToml<String>(U"entry_point");
		if (entryPoint == U"tutorial") goto tutorial;
		if (entryPoint == U"title") goto title;
		if (entryPoint == U"play") goto play;
		if (entryPoint == U"ending") goto ending;
		if (entryPoint == U"lounge") goto lounge;

		if (entryPoint == U"gamepad") (void)DialogGamepadRegister();
		if (entryPoint == U"yesno") (void)DialogYesNo(U"ねんね?");
		if (entryPoint == U"setting") (void)DialogSettingConfigure();
#endif

		fromTitle = tryLoadSavedata(); // セーブデータが存在するならタイトルから
		checkApplySteamLanguage(fromTitle); // Steam 言語の変更を反映 (初回起動時はセーブデータの書き込みは行わない)
		if (fromTitle) goto title;

	tutorial:
		tutorialLoop(yield, self, triedTutorial);
	title:
		triedTutorial = true;
		if (const bool retryTutorial = titleLoop(yield, self)) goto tutorial;
		if (getTemporaryRecord(m_savedata).floorIndex > 0) goto lounge;
	play:
		if (const bool clearedPlay = playLoop(yield, self)) goto ending;
	lounge:
		if (not loungeLoop(yield, self)) goto title;
		goto play;
	ending:
		endingLoop(yield, self);
		goto title;
	}

	// 反省: コルーチンにおける goto シーケンス管理でシーン遷移の可読性の向上を図ったか、思ったほど効果が得られなかった
	// 次回作からはイベントループ式シーン遷移に戻す

	bool tryLoadSavedata()
	{
		if (const auto d = LoadSavedata())
		{
			m_savedata = d.value();
			return true;
		}
		return false;
	}

	// Steam のクライアント言語を確認し、変更があれば反映する
	void checkApplySteamLanguage(bool canStoreSaveData)
	{
		const auto currentLanguage = GetSteamLanguage();
		if (currentLanguage == m_savedata.steamLanguage) return; // 変更なし

		GameConfig::Instance().language = currentLanguage;
		GameConfig::Instance().RequestWrite();

		m_savedata.steamLanguage = currentLanguage;
		if (canStoreSaveData) StoreSavedata(m_savedata);
	}

	void tutorialLoop(YieldExtended& yield, ActorView self, bool triedTutorial) const
	{
		auto tutorial = self.AsParent().Birth(Tutorial::TutorialScene());
		tutorial.Init(triedTutorial);
		yield.WaitForTrue([&]() { return tutorial.IsFinished(); });
		tutorial.Kill();
		StoreSavedata(m_savedata);
		yield();
	}

	bool titleLoop(YieldExtended& yield, ActorView self)
	{
		auto title = self.AsParent().Birth(Title::TitleScene());
		title.Init(m_savedata);
		if (0 < m_playData.floorIndex && m_playData.timeLimiter.remainingTime == 0)
		{
			// 遊んだ後なら死に戻り演出
			title.PerformReincarnate();
		}
		yield.WaitForTrue([&]()
		{
			return title.IsConcludedPlay() || title.IsConcludedRetryTutorial();
		});
		const bool retryTutorial = title.IsConcludedRetryTutorial();

		BgmManager::Instance().EndPlay();
		title.Kill();
		yield();

		initPlayData();

		return retryTutorial;
	}

	void initPlayData()
	{
		m_playData = {};
#if _DEBUG
		m_playData.floorIndex = debugToml<int>(U"initial_floor");
		if (m_playData.floorIndex <= 0) m_playData.floorIndex = 1;
		m_playData.timeLimiter = Play::TimeLimiterData{
			.maxTime = debugToml<double>(U"initial_timelimit"),
			.remainingTime = debugToml<double>(U"initial_timelimit"),
		};
#else
		m_playData.floorIndex = 1;
		m_playData.timeLimiter = {
			.maxTime = initialTimeLimit,
			.remainingTime = initialTimeLimit
		};
#endif
	}

	/// @return 50層を突破したなら true
	bool playLoop(YieldExtended& yield, ActorView self)
	{
		bool floorDown{};

		while (true)
		{
			auto playScene = self.AsParent().Birth(Play::PlayScene::Create());
			auto&& play = playScene.GetCore();

			// フロアに合わせたBGMを再生
			BgmManager::Instance().RequestPlay(Play::GetFloorData(m_playData.floorIndex));

#if _DEBUG
			if (not debugToml<bool>(U"skip_transition"))
#endif
			{
				// 遷移演出
				yield.WaitForExpire(
					play.StartTransition(m_playData.floorIndex, floorDown));
			}

			playScene.Init(m_playData);
			yield.WaitForTrue([&]()
			{
#if _DEBUG
				if (KeyNum9.down()) return true;
#endif
				return play.GetPlayer().IsTerminated();
			});
			if (not play.GetPlayer().IsTerminated()) yield();

			m_playData = play.CopyData();
			floorDown = play.GetPlayer().HasAbducted();

			playScene.Kill();
			yield();

#if _DEBUG
			if (debugToml<bool>(U"print_measured_times"))
			{
				Console.writeln(U"---");
				for (int i = 0; i < m_playData.measuredSeconds.size(); i++)
				{
					Console.writeln(U"{}F: {}"_fmt(i, m_playData.measuredSeconds[i]));
				}
			}
#endif

			if (m_playData.timeLimiter.remainingTime == 0)
			{
				// 回生の回廊へ
				requestSave(m_playData, false);

				BgmManager::Instance().EndPlay();
				return false;
			}
			if (not floorDown && m_playData.floorIndex == Constants::MaxFloor_50)
			{
				// エンディングへ
				requestSave(m_playData, true);

				BgmManager::Instance().EndPlay();
				yield.WaitForTime(2.0);
				return true;
			}
			else
			{
				// 次のフロアに移る前にセーブ
				requestSave(m_playData, false);
			}
#if _DEBUG
			if (not debugToml<bool>(U"constant_floor"))
#endif
			{
				if (floorDown) m_playData.floorIndex--;
				else m_playData.floorIndex++;
			}

			requestSave(m_playData, false);
			m_playData.timeLimiter.maxTime += 3;
			m_playData.timeLimiter.remainingTime += 3;
		}
	}

	/// @return タイトルに戻るなら false
	bool loungeLoop(YieldExtended& yield, ActorView self)
	{
		auto lounge = self.AsParent().Birth(Lounge::LoungeScene());
		lounge.Init({.reachedFloor = getTemporaryRecord(m_savedata).floorIndex});
		yield.WaitForTrue([&]() { return lounge.IsConcluded(); });
		lounge.Kill();
		yield();

		BgmManager::Instance().EndPlay();
		yield.WaitForTime(1.0);

		// タイトルに戻る
		if (lounge.IsReturnToTitle()) return false;

		// コンティニュー
		m_playData.floorIndex = lounge.NextFloor();
		if (m_playData.floorIndex == 1)
		{
			// 最初からのときは時間計測も初期化
			m_playData.measuredSeconds = {};
		}
		else
		{
			// 保存された時間計測を引き継ぐ
			m_playData.measuredSeconds = getTemporaryRecord(m_savedata).measuredSeconds;
		}

		const auto loungePlayData = lounge.GetPlayData();

		m_playData.playerPersonal.items = loungePlayData.playerPersonal.items;
		m_playData.timeLimiter = {.maxTime = initialTimeLimit, .remainingTime = initialTimeLimit};
		m_playData.itemIndexing = loungePlayData.itemIndexing;
		return true;
	}

	void requestSave(const Play::PlaySingletonData& data, bool isCleared)
	{
#if _DEBUG
		if (debugToml<bool>(U"no_save")) return;
#endif
		getReachedRecord(m_savedata).bestReached = Max(data.floorIndex, getReachedRecord(m_savedata).bestReached);

		const double previousCompletedTime = getReachedRecord(m_savedata).completedTime;
		if (isCleared && (data.measuredSeconds.Sum() < previousCompletedTime || previousCompletedTime == 0))
		{
			getReachedRecord(m_savedata).completedTime = data.measuredSeconds.Sum();
		}

		getTemporaryRecord(m_savedata).floorIndex = data.floorIndex;
		getTemporaryRecord(m_savedata).measuredSeconds = data.measuredSeconds;

		// セーブデータ更新
		StoreSavedata(m_savedata);

		// Steam に送信 (到達フロアより 1 つ小さい値を送信、ただしクリア時は 50 と送信)
		CheckStoreSteamStatOfCleared(
			Play::IsPlayingUra(),
			getReachedRecord(m_savedata).completedTime != 0 ? 50 : getReachedRecord(m_savedata).bestReached - 1
		);
	}

	void endingLoop(YieldExtended& yield, ActorView self) const
	{
		auto ending = self.AsParent().Birth(Ending::EndingScene());
		ending.Init(m_playData.measuredSeconds);
		yield.WaitForTrue([&]() { return ending.IsFinished(); });

		if (m_savedata.ura_record.bestReached == 0) DialogOk(U"announce_release_ura"_localize);

		ending.Kill();
	}
};

namespace Sukuu
{
	GamesSupervisor::GamesSupervisor() :
		p_impl(std::make_shared<Impl>())
	{
		StartCoro(*this, [*this](YieldExtended yield)
		{
			p_impl->FlowchartAsync(yield, *this);
		});
	}

	void GamesSupervisor::Update()
	{
		ActorBase::Update();
	}
}
