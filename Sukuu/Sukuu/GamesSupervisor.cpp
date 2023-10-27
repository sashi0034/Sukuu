#include "stdafx.h"
#include "GamesSupervisor.h"

#include "Play/PlayScene.h"
#include "Title/TitleScene.h"
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
		const auto entryPoint = getToml<String>(U"entry_point");
		if (entryPoint == U"title") goto title;
		if (entryPoint == U"play") goto play;

	title:
		{
			auto title = self.AsParent().Birth(Title::TitleScene());
			title.Init();
			yield.WaitForTrue([&]() { return title.IsConcluded(); });
			title.Kill();
		}
	play:
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
