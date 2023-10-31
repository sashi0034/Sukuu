#include "stdafx.h"
#include "TutorialScene.h"

#include "Constants.h"
#include "TutorialFocus.h"
#include "TutorialMap.h"
#include "TutorialMessenger.h"
#include "Play/PlayScene.h"
#include "Play/Enemy/EnKnight.h"
#include "Play/Enemy/EnSlimeCat.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"tutorial.scene." + key);
	}

	constexpr double messageWaitShortShort = 2.0;
	constexpr double messageWaitShort = 3.0;
	constexpr double messageWaitMedium = 4.0;
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
	TutorialFocus m_focus{};
	std::function<void()> m_postDraw{};

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
		auto&& timeLimiter = m_play.GetTimeLimiter();
		timeLimiter.SetCountEnabled(false);
		timeLimiter.SetImmortal(true);
		auto&& gimmick = m_play.GetGimmick();
		gimmick[m_mapData.itemSpawnPoint] = Play::GimmickKind::Item_Pin;
		gimmick[m_mapData.stairsPoint] = Play::GimmickKind::Stairs;

		m_messanger = self.AsParent().Birth(TutorialMessenger());
		m_focus = self.AsParent().Birth(TutorialFocus());
	}

	Play::MapGrid GetMap() const override
	{
		return std::move(m_mapData.map);
	}

	Vec2 InitialPlayerPos() const override
	{
		return m_mapData.initialPlayerPoint * Play::CellPx_24;
	}

	const Play::TutorialPlayerService& PlayerService() const override
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
		performOpening(yield, self);
		tutorialHowtoMove(yield);
		tutorialScoop(yield, self);
		tutorialItem(yield, self);
		m_playerService.canMove = true;
		tutorialFinal(yield, self);

		yield.WaitForTrue([this]() { return m_play.GetPlayer().IsTerminated(); });
		m_finished = true;
	}

	void performOpening(YieldExtended& yield, ActorView self)
	{
		// 真っ黒な画面から徐々に明るくしていく
		m_play.GetPlayer().PerformTutorialOpening();
		double rate = 1.0;
		m_postDraw = [&]()
		{
			Rect(Scene::Size()).draw(ColorF{Constants::HardDarkblue, rate});
		};
		yield.WaitForDead(
			AnimateEasing<EaseInQuad>(self, &rate, 0.0, 4.5));
		m_postDraw = {};
	}

	void tutorialHowtoMove(YieldExtended& yield)
	{
		m_playerService.canMove = false;
		m_play.GetMap().At(m_mapData.firstBlockPoint).kind = Play::TerrainKind::Wall;

		waitMessage(yield, U"ふむ、目が覚めたようだね", messageWaitMedium);
		waitMessage(yield, U"早速だがキミに動いてもらおう", messageWaitShort);
		m_playerService.canMove = true;
		int movedCount{};
		int runningCount{};
		m_playerService.onMoved = [&](auto, bool isRunning)
		{
			if (isRunning) runningCount++;
			movedCount++;
		};
		m_messanger.ShowMessageForever(U"まずは'W, A, S, D' で移動の確認をしてみようか");
		yield.WaitForTrue([&]
		{
			return movedCount > 10;
		});
		m_messanger.ShowMessageForever(U"次に 'Shift' を押しっぱなしにして走ってもらおう");
		yield.WaitForTrue([&]
		{
			return runningCount > 10;
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

		waitMessage(yield, U"おや、怪物に囲まれてしまったね", messageWaitMedium);
		waitMessage(yield, U"では、キミに運命を 'スクう' チカラを与えよう", messageWaitMedium);

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

		m_focus.Show(Scene::Center());
		auto scoopingMessage = StartCoro(self, [&](YieldExtended y)
		{
			waitMessage(y, U"キミ自身をマウスカーソルでスクってみるといい", messageWaitMedium);
			m_messanger.ShowMessageForever(U"自分自身を 'ドラッグ' して\n壁の方向へ 'ドロップ' するんだ");
		});
		yield.WaitForTrueVal(hasScooped);
		m_playerService.canScoop = false;

		timescaleController.Kill();
		scoopingMessage.Kill();
		m_focus.Hide();
		SetTimeScale(1.0);
		yield.WaitForTime(1.0);

		waitMessage(yield, U"間一髪、一難過ぎ去ったね", messageWaitShort);
		catNorth.Kill();
		catSouth.Kill();
		m_playerService.canMove = true;
		m_playerService.canScoop = true;
		m_playerService.onScooped = [](auto) { return; };
		m_playerService.canScoopTo = [&](auto) { return true; };
		waitMessage(yield, U"さて、奥へ進もうか", messageWaitShort);
	}

	void tutorialItem(YieldExtended& yield, ActorView self)
	{
		auto knight = m_play.GetEnemies().Birth(m_play.AsParent(), Play::EnKnight());
		knight.InitTutorial(m_mapData.knightSpawnPoint * Play::CellPx_24, Dir4::Left);

		bool nearItem{};
		m_playerService.onMoved = [&](const Play::CharaVec2& pos, bool isRunning)
		{
			if ((pos.MapPoint() - m_mapData.itemSpawnPoint).manhattanLength() <= 5) nearItem = true;
		};
		const auto leftRegionLimit = [this](const Play::CharaVec2& pos)
		{
			return pos.MapPoint().x < m_mapData.knightBlockPoint.x;
		};
		m_playerService.canMoveTo = leftRegionLimit;
		m_playerService.canScoopTo = leftRegionLimit;

		yield.WaitForTrueVal(nearItem);
		waitMessage(yield, U"イイものが落ちているね", messageWaitShortShort);
		waitMessage(yield, U"これを使ってみよう", messageWaitShortShort);

		yield.WaitForTrue([&]() { return knight.IsDead(); });

		m_playerService.onMoved = [](auto, auto) { return; };
		m_playerService.canMoveTo = [](auto) { return true; };
		m_playerService.canScoopTo = [](auto) { return true; };

		waitMessage(yield, U"うん、いいね", messageWaitShortShort);
	}

	void tutorialFinal(YieldExtended& yield, ActorView self)
	{
		bool nearStairs1{};
		m_playerService.onMoved = [&](const Play::CharaVec2& pos, bool isRunning)
		{
			if ((pos.MapPoint() - m_mapData.stairsPoint).manhattanLength() <= 4) nearStairs1 = true;
		};
		yield.WaitForTrueVal(nearStairs1);
		m_playerService.onMoved = [](auto, auto) { return; };

		m_playerService.canMove = false;
		m_playerService.canScoop = false;
		waitMessage(yield, U"最後に重要なことを話しておこう", messageWaitShort);
		waitMessage(yield, U"実はこの迷宮でキミが生きられる\n時間は限られている", messageWaitMedium);

		m_play.GetTimeLimiter().SetCountEnabled(true);
		m_focus.Show(Rect(Scene::Size()).tr() + Size{-1, 1} * 144);

		waitMessage(yield, U"砂時計を見てもらいたい", messageWaitShort);

		waitMessage(yield, U"これが0になるとキミは死を迎えるだろう", messageWaitShort);
		waitMessage(yield, U"気を付けるといい", messageWaitShortShort);

		m_focus.Hide();

		m_playerService.canMove = true;
		m_playerService.canScoop = true;

		bool nearStairs2{};
		m_playerService.onMoved = [&](const Play::CharaVec2& pos, bool isRunning)
		{
			if ((pos.MapPoint() - m_mapData.stairsPoint).manhattanLength() <= 2) nearStairs2 = true;
		};
		yield.WaitForTrueVal(nearStairs2);
		m_playerService.onMoved = [](auto, auto) { return; };

		m_playerService.canMove = false;
		waitMessage(yield, U"キミに語りかけられるのはここまでだ", messageWaitShort);
		m_playerService.canMove = true;
		waitMessage(yield, U"検討を祈っているよ", messageWaitShortShort);
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

	void TutorialScene::Update()
	{
		ActorBase::Update();
		if (p_impl->m_postDraw) p_impl->m_postDraw();
	}
}
