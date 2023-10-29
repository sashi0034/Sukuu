#include "stdafx.h"
#include "TutorialScene.h"

#include "TutorialMap.h"
#include "TutorialMessenger.h"
#include "Play/PlayScene.h"

namespace
{
}

struct Tutorial::TutorialScene::Impl : Play::ITutorialSetting
{
	Play::PlayScene m_play{};
	TutorialMapData m_mapData{};
	bool m_finished{};
	Play::TutorialPlayerService m_playerService{
		.canMove = false,
		.canScoop = false,
		.onMove = [](auto) { return; },
		.onScoop = [](auto) { return; },
		.canMoveTo = [](auto) { return true; },
		.canScoopTo = [](auto) { return true; },
	};
	TutorialMessenger m_messanger{};

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
		m_messanger = self.AsParent().Birth(TutorialMessenger());
	}

	Play::MapGrid GetMap() const override
	{
		return std::move(m_mapData.map);
	}

	Vec2 InitialPlayerPos() const override
	{
		return m_mapData.initialPlayerPoint * Play::CellPx_24;
	}

	const Play::TutorialPlayerService& PlayerService() const
	{
		return m_playerService;
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
		m_play.GetMap().At(m_mapData.firstBlockPoint).kind = Play::TerrainKind::Wall;
		m_playerService.canMove = true;

		while (true)
		{
			m_messanger.ShowMessageForever(U"テストメッセージ");
			yield.WaitForTime(4.0);

			m_messanger.HideMessage();
			yield.WaitForTime(1.0);
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
