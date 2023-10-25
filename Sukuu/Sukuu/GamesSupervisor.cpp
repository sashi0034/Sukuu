#include "stdafx.h"
#include "GamesSupervisor.h"

#include "Play/PlayScene.h"
#include "Util/CoroUtil.h"

using namespace Util;

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
	void flowchartLoop(YieldExtended& yield, ActorBase& self)
	{
		auto play = self.AsParent().Birth(Play::PlayScene(m_playData));
		yield.WaitForTrue([&]()
		{
			return play.GetPlayer().IsCompletedGoal();
		});
		play.Kill();
		m_playData = play.CopyData();
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
