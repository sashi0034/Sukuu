#include "stdafx.h"
#include "GamesSupervisor.h"

#include "Constants.h"
#include "GameSavedata.h"
#include "Ending/EndingScene.h"
#include "Play/PlayBgm.h"
#include "Play/PlayScene.h"
#include "Play/Other/FloorLevelDesign.h"
#include "Title/TitleScene.h"
#include "Tutorial/TutorialScene.h"
#include "Util/CoroUtil.h"

namespace
{
	using namespace Util;

	template <typename T>
	inline T debugToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"debug." + key);
	}
}

struct Sukuu::GamesSupervisor::Impl
{
	void FlowchartAsync(YieldExtended& yield, ActorView self)
	{
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
#if _DEBUG
		const auto entryPoint = debugToml<String>(U"entry_point");
		if (entryPoint == U"tutorial") goto tutorial;
		if (entryPoint == U"title") goto title;
		if (entryPoint == U"play") goto play;
		if (entryPoint == U"ending") goto ending;
#endif

		if (tryLoadSavedata()) goto title;

	tutorial:
		tutorialLoop(yield, self);
	title:
		if (const bool retryTutorial = titleLoop(yield, self)) goto tutorial;
	play:
		if (const bool clearedPlay = playLoop(yield, self)) goto ending;
		goto title;
	ending:
		endingLoop(yield, self);
		goto title;
	}

	bool tryLoadSavedata()
	{
		if (const auto d = LoadSavedata())
		{
			m_savedata = d.value();
			return true;
		}
		return false;
	}

	void tutorialLoop(YieldExtended& yield, ActorView self) const
	{
		auto tutorial = self.AsParent().Birth(Tutorial::TutorialScene());
		tutorial.Init();
		yield.WaitForTrue([&]() { return tutorial.IsFinished(); });
		tutorial.Kill();
		SaveSavedata(m_savedata);
		yield();
	}

	bool titleLoop(YieldExtended& yield, ActorView self) const
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
			return dialogRetryTutorial() || title.IsConcluded();
		});
		const bool retryTutorial = not title.IsConcluded();
		title.Kill();
		yield();
		return retryTutorial;;
	}

	static bool dialogRetryTutorial()
	{
		if (KeyLControl.pressed() && KeyR.down())
		{
			const bool yes =
				System::MessageBoxYesNo(U"もう一度チュートリアルをしますか?", MessageBoxStyle::Question) == MessageBoxResult::Yes;
			return yes;
		}
		return false;
	}

	bool playLoop(YieldExtended& yield, ActorView self)
	{
		m_playData = {};
#if _DEBUG
		m_playData.floorIndex = debugToml<int>(U"initial_floor");
		m_playData.timeLimiter = Play::TimeLimiterData{
			.maxTime = debugToml<double>(U"initial_timelimit"),
			.remainingTime = debugToml<double>(U"initial_timelimit"),
		};
#else
		m_playData.floorIndex = 1;
		m_playData.timeLimiter = {
			.maxTime = 90.0,
			.remainingTime = 90.0,
		};
#endif

		while (true)
		{
			auto play = self.AsParent().Birth(Play::PlayScene::Create());

			// フロアに合わせたBGMを再生
			Play::PlayBgm::Instance().RequestPlay(Play::GetFloorBgm(m_playData.floorIndex));

#if _DEBUG
			if (not debugToml<bool>(U"skip_transition"))
#endif
			{
				// 遷移演出
				yield.WaitForExpire(
					play.StartTransition(m_playData.floorIndex));
			}

			play.Init(m_playData);
			yield.WaitForTrue([&]()
			{
#if _DEBUG
				if (KeyNum9.down()) return true;
#endif
				return play.GetPlayer().IsTerminated();
			});
			if (not play.GetPlayer().IsTerminated()) yield();
			m_playData = play.CopyData();
			play.Kill();
			yield();
			if (m_playData.timeLimiter.remainingTime == 0)
			{
				// ゲームオーバー
				return false;
			}
			if (m_playData.floorIndex == Constants::MaxFloorIndex)
			{
				// エンディング
				Play::PlayBgm::Instance().EndPlay();
				checkSave(m_playData, true);
				return true;
			}
#if _DEBUG
			if (not debugToml<bool>(U"constant_floor")) m_playData.floorIndex++;
#else
			m_playData.floorIndex++;
#endif
			checkSave(m_playData, false);
			m_playData.timeLimiter.maxTime += 3;
			m_playData.timeLimiter.remainingTime += 3;
		}
	}

	void checkSave(const Play::PlaySingletonData& data, bool isCleared)
	{
		const auto newData = GameSavedata{
			.bestReached = data.floorIndex,
			.completedTime = isCleared ? data.measuredSeconds.Sum() : 0
		};

		// 到達フロア更新
		const bool updatedReached = newData.bestReached > m_savedata.bestReached;

		// クリア時間更新
		const bool updatedCleared =
			newData.bestReached == 50
			&& 0 < newData.completedTime
			&& (m_savedata.completedTime == 0 || newData.completedTime < m_savedata.completedTime);

		if (updatedReached || updatedCleared)
		{
			SaveSavedata(newData);
			m_savedata = newData;
		}
	}

	void endingLoop(YieldExtended& yield, ActorView self) const
	{
		auto ending = self.AsParent().Birth(Ending::EndingScene());
		ending.Init(m_playData.measuredSeconds);
		yield.WaitForTrue([&]() { return ending.IsFinished(); });
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
