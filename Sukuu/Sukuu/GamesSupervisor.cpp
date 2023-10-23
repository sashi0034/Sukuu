#include "stdafx.h"
#include "GamesSupervisor.h"

#include "Play/PlayScene.h"
#include "Util/CoroUtil.h"

using namespace Util;

struct Sukuu::GamesSupervisor::Impl
{
	void FlowchartAsync(YieldExtended& yield, ActorBase& self) const
	{
		while (true)
		{
			flowchartLoop(yield, self);
		}
	}

private:
	static void flowchartLoop(YieldExtended& yield, ActorBase& self)
	{
		auto play = self.AsParent().Birth(Play::PlayScene());
		yield.WaitForTrue([&]()
		{
			return play.GetPlayer().IsCompletedGoal();
		});
		play.Kill();
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
