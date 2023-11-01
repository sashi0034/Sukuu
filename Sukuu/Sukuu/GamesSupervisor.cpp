#include "stdafx.h"
#include "GamesSupervisor.h"

#include "Constants.h"
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
	Play::PlaySingletonData m_playData{};

	void FlowchartAsync(YieldExtended& yield, ActorBase& self)
	{
		while (true)
		{
			flowchartLoop(yield, self);
		}
	}

private:
	void flowchartLoop(YieldExtended& yield, ActorView self)
	{
#if _DEBUG
		const auto entryPoint = debugToml<String>(U"entry_point");
		if (entryPoint == U"tutorial") goto tutorial;
		if (entryPoint == U"title") goto title;
		if (entryPoint == U"play") goto play;
		if (entryPoint == U"ending") goto ending;
#endif

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

	void tutorialLoop(YieldExtended& yield, ActorView self)
	{
		auto tutorial = self.AsParent().Birth(Tutorial::TutorialScene());
		tutorial.Init();
		yield.WaitForTrue([&]() { return tutorial.IsFinished(); });
		tutorial.Kill();
	}

	void titleLoop(YieldExtended& yield, ActorView self)
	{
		auto title = self.AsParent().Birth(Title::TitleScene());
		title.Init();
		yield.WaitForTrue([&]() { return title.IsConcluded(); });
		title.Kill();
	}

	bool playLoop(YieldExtended& yield, ActorView self)
	{
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
			auto play = self.AsParent().Birth(Play::PlayScene());
#if _DEBUG
			if (not debugToml<bool>(U"skip_transition"))
#endif
			{
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
			play.Kill();
			m_playData = play.CopyData();
			if (m_playData.timeLimiter.remainingTime == 0)
			{
				// ゲームオーバー
				return false;
			}
			if (m_playData.floorIndex == Constants::MaxFloorIndex)
			{
				// エンディング
				return true;
			}
#if _DEBUG
			if (not debugToml<bool>(U"constant_floor")) m_playData.floorIndex++;
#else
			m_playData.floorIndex++;
#endif
			m_playData.timeLimiter.maxTime += 3;
			m_playData.timeLimiter.remainingTime += 3;
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
