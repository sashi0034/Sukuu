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
#include "Play/PlayBgm.h"
#include "Play/PlayingUra.h"
#include "Play/PlayScene.h"
#include "Play/Other/FloorLevelDesign.h"
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
		checkApplySteamLanguage(); // Steam 言語の変更を反映
		if (fromTitle) goto title;

	tutorial:
		tutorialLoop(yield, self, triedTutorial);
	title:
		triedTutorial = true;
		if (const bool retryTutorial = titleLoop(yield, self)) goto tutorial;
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
	void checkApplySteamLanguage()
	{
		const auto currentLanguage = GetSteamLanguage();
		if (currentLanguage == m_savedata.steamLanguage) return; // 変更なし

		GameConfig::Instance().language = currentLanguage;
		GameConfig::Instance().RequestWrite();

		m_savedata.steamLanguage = currentLanguage;
		SaveSavedata(m_savedata);
	}

	void tutorialLoop(YieldExtended& yield, ActorView self, bool triedTutorial) const
	{
		auto tutorial = self.AsParent().Birth(Tutorial::TutorialScene());
		tutorial.Init(triedTutorial);
		yield.WaitForTrue([&]() { return tutorial.IsFinished(); });
		tutorial.Kill();
		SaveSavedata(m_savedata);
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
			Play::PlayBgm::Instance().RequestPlay(Play::GetFloorBgm(m_playData.floorIndex));

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

			checkSave(m_playData, false);

			if (m_playData.timeLimiter.remainingTime == 0)
			{
				// 回生の回廊へ
				return false;
			}
			if (m_playData.floorIndex == Constants::MaxFloorIndex)
			{
				// エンディングへ
				Play::PlayBgm::Instance().EndPlay();
				return true;
			}
#if _DEBUG
			if (not debugToml<bool>(U"constant_floor"))
#endif
			{
				if (floorDown) m_playData.floorIndex--;
				else m_playData.floorIndex++;
			}
			checkSave(m_playData, false);
			m_playData.timeLimiter.maxTime += 3;
			m_playData.timeLimiter.remainingTime += 3;
		}
	}

	/// @return タイトルに戻るなら false
	bool loungeLoop(YieldExtended& yield, ActorView self)
	{
		auto lounge = self.AsParent().Birth(Lounge::LoungeScene());
		lounge.Init({.reachedFloor = m_playData.floorIndex});
		yield.WaitForTrue([&]() { return lounge.IsConcluded(); });
		lounge.Kill();
		yield();

		// タイトルに戻る
		if (lounge.IsReturnToTitle()) return false;

		// コンティニュー
		m_playData.floorIndex = lounge.NextFloor();
		if (m_playData.floorIndex == 1)
		{
			// 最初からのときは時間計測も初期化
			m_playData.measuredSeconds = {};
		}
		m_playData.playerPersonal = {};
		m_playData.timeLimiter = {.maxTime = initialTimeLimit, .remainingTime = initialTimeLimit};
		m_playData.itemIndexing = {};
		return true;
	}

	void checkSave(const Play::PlaySingletonData& data, bool isCleared)
	{
		const auto newRecord = ReachedRecord{
			.bestReached = data.floorIndex,
			.completedTime = isCleared ? data.measuredSeconds.Sum() : 0
		};
		auto newData = GameSavedata(m_savedata);
		getReachedRecord(newData) = newRecord;

		const auto savedRecord = getReachedRecord(m_savedata);

		// 到達フロア更新
		const bool updatedReached = newRecord.bestReached > savedRecord.bestReached;

		// クリア時間更新
		const bool updatedCleared =
			newRecord.bestReached == 50
			&& 0 < newRecord.completedTime
			&& (savedRecord.completedTime == 0 || newRecord.completedTime < savedRecord.completedTime);

		if (updatedReached || updatedCleared)
		{
			// セーブデータ更新
			SaveSavedata(newData);
			m_savedata = newData;
		}

		// Steam に送信
		CheckStoreSteamStatOfReach(Play::IsPlayingUra(), newRecord.bestReached);
	}

	static ReachedRecord& getReachedRecord(GameSavedata& data)
	{
		return data.GetRecord(Play::IsPlayingUra());
	}

	void endingLoop(YieldExtended& yield, ActorView self) const
	{
		auto ending = self.AsParent().Birth(Ending::EndingScene());
		ending.Init(m_playData.measuredSeconds);
		yield.WaitForTrue([&]() { return ending.IsFinished(); });

		if (m_savedata.ura.bestReached == 0) DialogOk(U"announce_release_ura"_localize);

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
		Play::PlayBgm::Instance().Refresh();
	}
}
