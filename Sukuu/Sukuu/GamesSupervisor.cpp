#include "stdafx.h"
#include "GamesSupervisor.h"

#include "Play/PlayScene.h"
#include "Title/TitleScene.h"
#include "Tutorial/TutorialScene.h"
#include "Util/CoroUtil.h"

namespace
{
	using namespace Util;

	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"sukuu." + key);
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
		const auto entryPoint = getToml<String>(U"entry_point");
		if (entryPoint == U"tutorial") goto tutorial;
		if (entryPoint == U"title") goto title;
		if (entryPoint == U"play") goto play;

	tutorial:
		tutorialLoop(yield, self);
	title:
		titleLoop(yield, self);
	play:
		playLoop(yield, self);
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

	void playLoop(YieldExtended& yield, ActorView self)
	{
		m_playData.floorIndex = 1;
		m_playData.timeLimiter = {
			.maxTime = 120,
			.remainingTime = 120,
		};

		while (true)
		{
			auto play = self.AsParent().Birth(Play::PlayScene());
			yield.WaitForExpire(play.StartTransition(m_playData.floorIndex));
			play.Init(m_playData);
			yield.WaitForTrue([&]()
			{
				return play.GetPlayer().IsCompletedGoal();
			});
			play.Kill();
			m_playData = play.CopyData();
			m_playData.floorIndex++;
		}
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
