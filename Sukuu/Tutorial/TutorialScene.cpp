#include "stdafx.h"
#include "TutorialScene.h"

#include "TutorialMap.h"
#include "Play/PlayScene.h"

namespace
{
}

struct Tutorial::TutorialScene::Impl : Play::ITutorialSetting
{
	Play::PlayScene m_play{};
	TutorialMapData m_mapData{};
	bool m_finished{};

	void Init(ActorView self)
	{
		m_mapData = GetTutorialMap();
		m_play = self.AsParent().Birth(Play::PlayScene());
		m_play.Init({
			.tutorial = this,
			.playerPersonal = {},
			.timeLimiter = Play::TimeLimiterData{
				.maxTime = 60,
				.remainingTime = 60
			}
		});
	}

	Play::MapGrid GetMap() const override
	{
		return std::move(m_mapData.map);
	}

	Vec2 InitialPlayerPos() const override
	{
		return m_mapData.initialPlayerPoint * Play::CellPx_24;
	}

	void StartFlowchart(ActorView self)
	{
		StartCoro(self, [self, this](YieldExtended yield)
		{
			flowchartLoop(yield, self);
		});
	}

private:
	void flowchartLoop(YieldExtended& yield, ActorView self)
	{
		while (true)
		{
			yield();
		}
	}
};

namespace Tutorial
{
	TutorialScene::TutorialScene() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void TutorialScene::Init()
	{
		p_impl->Init(*this);
		p_impl->StartFlowchart(*this);
	}

	bool TutorialScene::IsFinished() const
	{
		return p_impl->m_finished;
	}
}
