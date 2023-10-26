#include "stdafx.h"
#include "GamesSupervisor.h"

#include "Play/PlayScene.h"
#include "Title/TitleScene.h"
#include "Util/CoroUtil.h"

using namespace Util;

struct Sukuu::GamesSupervisor::Impl
{
	Play::PlaySingletonData m_playData{};

	void FlowchartAsync(YieldExtended& yield, ActorBase& self)
	{
		m_playData.timeLimiter = {
			.maxTime = 120,
			.remainingTime = 120,
		};

		while (true)
		{
			flowchartLoop(yield, self);
		}
	}

private:
	void flowchartLoop(YieldExtended& yield, ActorBase& self)
	{
		auto title = self.AsParent().Birth(Title::TitleScene());
		title.Init();
		yield.WaitForTrue([&]() { return title.IsConcluded(); });
		title.Kill();

		while (true)
		{
			auto play = self.AsParent().Birth(Play::PlayScene());
			play.Init(m_playData);
			yield.WaitForTrue([&]()
			{
				return play.GetPlayer().IsCompletedGoal();
			});
			play.Kill();
			m_playData = play.CopyData();
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
