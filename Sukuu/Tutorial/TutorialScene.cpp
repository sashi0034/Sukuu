#include "stdafx.h"
#include "TutorialScene.h"

#include "TutorialMap.h"
#include "TutorialMessenger.h"
#include "Play/PlayScene.h"
#include "Play/Enemy/EnSlimeCat.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"tutorial.scene." + key);
	}

	constexpr double messageWaitShort = 4.0;
	constexpr double messageWaitMedium = 6.0;
	constexpr double messageWaitLong = 8.0;
}

struct Tutorial::TutorialScene::Impl : Play::ITutorialSetting
{
	Play::PlayScene m_play{};
	TutorialMapData m_mapData{};
	bool m_finished{};
	Play::TutorialPlayerService m_playerService{
		.canMove = false,
		.canScoop = false,
		.onMoved = [](auto, auto) { return; },
		.onScooped = [](auto) { return; },
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
	void waitMessage(YieldExtended& yield, const String& message, double duration)
	{
		m_messanger.ShowMessage(message, duration);
		yield.WaitForTime(duration, Scene::DeltaTime);
		yield();
	}

	void flowchartLoop(YieldExtended& yield, ActorView self)
	{
		tutorialHowtoMove(yield);
		m_playerService.canMove = true;

		tutorialScoop(yield, self);

		waitMessage(yield, U"Not implemented", messageWaitShort);
	}

	void tutorialHowtoMove(YieldExtended& yield)
	{
		m_play.GetMap().At(m_mapData.firstBlockPoint).kind = Play::TerrainKind::Wall;

		waitMessage(yield, U"ふむ、目が覚めたようだね", messageWaitShort);
		waitMessage(yield, U"早速だがキミに動いてもらおう", messageWaitShort);
		m_playerService.canMove = true;
		int movedCount{};
		int runningCount{};
		m_playerService.onMoved = [&](auto, bool isRunning)
		{
			if (isRunning) runningCount++;
			movedCount++;
		};
		waitMessage(yield, U"まずは'W, A, S, D' で移動の確認をしてみようか", messageWaitMedium);
		yield.WaitForTrue([&]
		{
			return movedCount > 5;
		});
		waitMessage(yield, U"次に 'Shift' を押しっぱなしにして走ってもらおう", messageWaitMedium);
		yield.WaitForTrue([&]
		{
			return runningCount > 5;
		});
		waitMessage(yield, U"よし、では奥に進もうか", messageWaitShort);

		// とうせんぼうしてたブロックを除去
		m_play.GetMap().At(m_mapData.firstBlockPoint).kind = Play::TerrainKind::Floor;

		m_playerService.onMoved = [&](auto, auto) { return; };
	}

	void tutorialScoop(YieldExtended& yield, ActorView self)
	{
		m_playerService.onMoved = [&](const Play::CharaVec2& pos, auto)
		{
			if (pos.MapPoint() == m_mapData.sukuuEventPoint)
			{
				m_playerService.canMove = false;
			}
		};
		yield.WaitForFalseVal(m_playerService.canMove);
		m_playerService.onMoved = [](auto, auto) { return; };

		// すくう入手イベント発生
		auto catSouth = m_play.GetEnemies().Birth(m_play.AsParent(), Play::EnSlimeCat());
		catSouth.InitTutorial(m_mapData.catSpawnPoint_South * Play::CellPx_24, Dir4::Up);

		auto catNorth = m_play.GetEnemies().Birth(m_play.AsParent(), Play::EnSlimeCat());
		catNorth.InitTutorial(m_mapData.catSpawnPoint_North * Play::CellPx_24, Dir4::Down);

		const auto catNorthDist = [&]()
		{
			return (catNorth.Pos().actualPos - m_play.GetPlayer().CurrentPos().actualPos).manhattanLength();
		};

		// ネコとの距離が近くなるまで待機
		constexpr int findableCell = 6;
		yield.WaitForTrue([&]()
		{
			return catNorthDist() < Play::CellPx_24 * findableCell;
		});

		auto timescaleController = StartCoro(self, [&](YieldExtended yield1)
		{
			// ネコが近づくたびにスローになるように
			while (true)
			{
				constexpr int e1 = 2;
				constexpr int e2 = (findableCell - e1);
				const double timescale =
					Math::Clamp(EaseInExpo((catNorthDist() - Play::CellPx_24 * e1) / (Play::CellPx_24 * e2)),
					            0.01, 1.0);
				SetTimeScale(timescale);
				yield1();
			}
		});

		waitMessage(yield, U"おや、物の怪に挟まれてしまったね", messageWaitShort);
		waitMessage(yield, U"では、キミに運命を 'スクう' チカラを与えよう", messageWaitShort);

		bool hasScooped{};
		m_playerService.canScoop = true;
		m_playerService.canScoopTo = [&](const Play::CharaVec2& pos)
		{
			// 左右だけすくえるようにする
			return m_play.GetPlayer().CurrentPos().actualPos.MapPoint().y == pos.MapPoint().y;
		};
		m_playerService.onScooped = [&](auto)
		{
			hasScooped = true;
		};

		auto scoopingMessage = StartCoro(self, [&](YieldExtended y)
		{
			waitMessage(y, U"キミ自身をマウスカーソルでスクってみるといい", messageWaitMedium);
			m_messanger.ShowMessageForever(U"自分自身を 'ドラッグ' して\n壁の方向へ 'ドロップ' するんだ");
		});
		yield.WaitForTrueVal(hasScooped);
		m_playerService.canScoop = false;

		timescaleController.Kill();
		scoopingMessage.Kill();
		SetTimeScale(1.0);
		yield.WaitForTime(1.0);

		waitMessage(yield, U"一難が過ぎ去ったみたいだね", messageWaitMedium);
		catNorth.Kill();
		catSouth.Kill();
		m_playerService.canMove = true;
		m_playerService.canScoop = true;
		m_playerService.onScooped = [](auto) { return; };
		m_playerService.canScoopTo = [&](auto) { return true; };
		waitMessage(yield, U"さて、奥へ進もうか", messageWaitMedium);
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
