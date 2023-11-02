#include "stdafx.h"
#include "GamesSupervisor.h"

#include "Constants.h"
#include "GameSavedata.h"
#include "Ending/EndingScene.h"
#include "Play/PlayScene.h"
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
	void FlowchartAsync(YieldExtended& yield, ActorBase& self)
	{
		while (true)
		{
			flowchartLoop(yield, self);
		}
	}

private:
	GameSavedata m_savedata{};

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
		titleLoop(yield, self);
	play:
		if (const bool cleared = playLoop(yield, self)) goto ending;
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
	}

	void titleLoop(YieldExtended& yield, ActorView self) const
	{
		auto title = self.AsParent().Birth(Title::TitleScene());
		title.Init(m_savedata);
		yield.WaitForTrue([&]() { return title.IsConcluded(); });
		title.Kill();
	}

	bool playLoop(YieldExtended& yield, ActorView self)
	{
#if _DEBUG
		Play::PlaySingletonData playData{};
		playData.floorIndex = debugToml<int>(U"initial_floor");
		playData.timeLimiter = Play::TimeLimiterData{
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
			auto play = self.AsParent().Birth(Play::PlayScene());
#if _DEBUG
			if (not debugToml<bool>(U"skip_transition"))
#endif
			{
				yield.WaitForExpire(
					play.StartTransition(playData.floorIndex));
			}
			play.Init(playData);
			yield.WaitForTrue([&]()
			{
#if _DEBUG
				if (KeyNum9.down()) return true;
#endif
				return play.GetPlayer().IsTerminated();
			});
			if (not play.GetPlayer().IsTerminated()) yield();
			play.Kill();
			playData = play.CopyData();
			checkSave(playData);
			if (playData.timeLimiter.remainingTime == 0)
			{
				// ゲームオーバー
				return false;
			}
			if (playData.floorIndex == Constants::MaxFloorIndex)
			{
				// エンディング
				return true;
			}
#if _DEBUG
			if (not debugToml<bool>(U"constant_floor")) playData.floorIndex++;
#else
			m_playData.floorIndex++;
#endif
			playData.timeLimiter.maxTime += 3;
			playData.timeLimiter.remainingTime += 3;
		}
	}

	void checkSave(const Play::PlaySingletonData& data)
	{
		const auto newData = GameSavedata{
			.bestReached = data.floorIndex,
			.completedTime = 0 // TODO
		};
		if (newData.bestReached > m_savedata.bestReached ||
			(newData.bestReached == 50 && newData.completedTime < m_savedata.completedTime))
		{
			SaveSavedata(newData);
			m_savedata = newData;
		}
	}

	void endingLoop(YieldExtended& yield, ActorView self)
	{
		auto ending = self.AsParent().Birth(Ending::EndingScene());
		ending.Init();
		yield.WaitForTrue([&]() { return ending.IsFinished(); });
		ending.Kill();
	}
};

namespace Sukuu
{
	GamesSupervisor::GamesSupervisor() :
		p_impl(std::make_shared<Impl>())
	{
		StartCoro(*this, [*this](YieldExtended yield) mutable
		{
			p_impl->FlowchartAsync(yield, *this);
		});
	}

	void GamesSupervisor::Update()
	{
		ActorBase::Update();
	}
}
