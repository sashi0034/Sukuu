#include "stdafx.h"
#include "TutorialScene.h"

#include "AssetKeys.h"
#include "Assets.generated.h"
#include "Constants.h"
#include "TutorialFocus.h"
#include "TutorialMap.h"
#include "TutorialMessenger.h"
#include "Gm/GameCursor.h"
#include "Gm/GamepadObserver.h"
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

struct Tutorial::TutorialScene::Impl
{
	Play::PlayScene m_playScene{Play::PlayScene::Empty()};
	Play::PlayCore m_play{Play::PlayCore::Empty()};
	TutorialMapData m_mapData{};
	bool m_finished{};
	TutorialMessenger m_messanger{};
	TutorialFocus m_focus{};
	std::function<void()> m_postDraw{};
	AudioAsset m_bgm = AudioAsset(AssetBgms::obake_dance);
	bool m_retrying{};

	void Init(ActorView self)
	{
		m_playScene = self.AsParent().Birth(Play::PlayScene::Create());
		m_play = m_playScene.GetCore();

		// フロー開始
		StartCoro(m_play.AsMainContent(), [this](YieldExtended yield)
		{
			flowchartLoop(yield);
		});
	}

private:
	Play::PlayerExportedService& playerServices()
	{
		return m_playScene.GetCore().GetPlayer().ExportedService();
	}

	void startPlayScene()
	{
		m_mapData = GetTutorialMap();

		// PlayScene 初期化
		m_playScene.Init({
			.designatedMap = Play::DesignatedMapInfo{
				.map = m_mapData.map,
				.initialPlayerPos = m_mapData.initialPlayerPoint * Play::CellPx_24
			},
			.playerPersonal = {},
			.timeLimiter = Play::TimeLimiterData{
				.maxTime = 90,
				.remainingTime = 60
			}
		});

		// プレイヤーの設定
		playerServices().forcedImmortal = true;
		playerServices().canMove = false;
		playerServices().canScoop = false;

		// タイムリミットの設定
		auto&& timeLimiter = m_play.GetTimeLimiter();
		timeLimiter.SetCountEnabled(false);
		timeLimiter.SetImmortal(true);

		// チュートリアル用ギミック設置
		auto&& gimmick = m_play.GetGimmick();
		gimmick[m_mapData.itemSpawnPoint] = Play::GimmickKind::Item_Pin;
		gimmick[m_mapData.stairsPoint] = Play::GimmickKind::Stairs;
		for (auto&& p : m_mapData.hourglassPoints)
		{
			gimmick[p] = Play::GimmickKind::SemiItem_Hourglass;
		}

		m_messanger = m_play.AsUiContent().Birth(TutorialMessenger());
		m_focus = m_play.AsUiContent().Birth(TutorialFocus());
	}

	void waitMessage(YieldExtended& yield, const String& message, double duration)
	{
		m_messanger.ShowMessage(message, duration);
		yield.WaitForTime(duration, Scene::DeltaTime);
		yield();
	}

	void flowchartLoop(YieldExtended& yield)
	{
		yield();
		if (not m_retrying) performLogo(yield);
		startPlayScene();

		performOpening(yield);

		tutorialHowtoMove(yield);

		tutorialScoop(yield);

		tutorialItem(yield);

		playerServices().canMove = true;

		tutorialFinal(yield);

		yield.WaitForTrue([this]() { return m_play.GetPlayer().IsTerminated(); });
		m_bgm.stop(3.0s);
		yield.WaitForTime(3.0);
		m_finished = true;
	}

	void performLogo(YieldExtended& yield)
	{
		const auto icon = TextureAsset(AssetImages::siv3d_icon);
		const auto font = FontAsset(AssetKeys::RocknRoll_Sdf);

		double fadeTransition{};
		m_postDraw = [&]()
		{
			Rect(Scene::Size()).draw(ColorF{Constants::HardDarkblue});
			Rect(Scene::Size()).draw(ColorF(1.0, 1.0 - fadeTransition));

			Transformer2D t0{
				Mat3x2::Rotate(2 * 360_deg * EaseInBack(fadeTransition), Scene::Center()).
				scaled(1 + 127 * EaseInBack(fadeTransition), Scene::Center()),
				TransformCursor::No
			};

			Transformer2D t1{
				Mat3x2::Translate(32 * Periodic::Sine1_1(2.0s), 8 * Periodic::Sine1_1(3.0s)),
				TransformCursor::No
			};

			// Siv3Dロゴ
			icon.resized(800, 800)
			    .drawAt(Scene::Center());

			font(U"左クリックかAボタンで開始")
				.drawAt(32.0 + 8.0 * Periodic::Sine0_1(3.0s), Scene::Center().moveBy(0, 400), ColorF(U"#2591d4"));
		};

		yield.WaitForTime(0.5);
		yield.WaitForTrue([] { return Gm::CheckConfirmSimply(); });

		// 開始
		AudioAsset(AssetSes::floor_transition).playOneShot();

		yield.WaitForExpire(AnimateEasing<EaseInLinear>(m_play.AsMainContent(), &fadeTransition, 1.0, 1.5));
		m_postDraw = {};
	}

	void performOpening(YieldExtended& yield)
	{
		m_play.GetPause().SetAllowed(false);
		m_bgm.setLoop(true);
		m_bgm.play(Constants::BgmMixBus);

		double prologueAlpha{};
		const auto prologueFont = FontAsset(AssetKeys::RocknRoll_Sdf);
		m_postDraw = [&]()
		{
			Rect(Scene::Size()).draw(ColorF{Constants::HardDarkblue});
			prologueFont(U"ダンジョン50層先のいにしえの地を求めて...").drawAt(
				40.0, Scene::Center(), ColorF(1.0, prologueAlpha));
		};
		yield.WaitForExpire(AnimateEasing<EaseInOutSine>(m_play.AsMainContent(), &prologueAlpha, 1.0, 2.0));
		yield.WaitForTime(3.0);
		yield.WaitForExpire(AnimateEasing<EaseInOutSine>(m_play.AsMainContent(), &prologueAlpha, 0.0, 2.0));

		// 真っ黒な画面から徐々に明るくしていく
		m_play.GetPlayer().PerformTutorialOpening();
		double rate = 1.0;
		m_postDraw = [&]()
		{
			Rect(Scene::Size()).draw(ColorF{Constants::HardDarkblue, rate});
		};
		yield.WaitForExpire(
			AnimateEasing<EaseInQuad>(m_play.AsMainContent(), &rate, 0.0, 8.0));
		m_postDraw = {};
		m_play.GetPause().SetAllowed(true);
	}

	void tutorialHowtoMove(YieldExtended& yield)
	{
		playerServices().canMove = false;
		m_play.GetMap().At(m_mapData.firstBlockPoint).kind = Play::TerrainKind::Wall;

		yield.WaitForTime(2.0);
		waitMessage(yield, U"目が覚めたようだね", messageWaitMedium);
		waitMessage(yield, U"早速だがキミに動いてもらおう", messageWaitMedium);
		playerServices().canMove = true;
		int movedCount{};
		int runningCount{};
		playerServices().onMoved = [&](auto, bool isRunning)
		{
			if (isRunning) runningCount++;
			movedCount++;
		};
		m_messanger.ShowMessageForever(U"まずは {} で移動の確認をしてみようか"_fmt(
			Gm::IsUsingGamepad()
				? U"'十字キー'"
				: U"'W, A, S, D' か `矢印キー`"));
		yield.WaitForTrue([&]
		{
			return movedCount > 10;
		});
		m_messanger.ShowMessageForever(U"次に {} を押しっぱなしにして走ってもらおう"_fmt(
			Gm::IsUsingGamepad()
				? U"'B'"
				: U"'Shift'"));
		yield.WaitForTrue([&]
		{
			return runningCount > 10;
		});
		waitMessage(yield, U"よし、では奥に進もうか", messageWaitMedium);

		// とうせんぼうしてたブロックを除去
		AudioAsset(AssetSes::attack2).playOneShot();
		m_play.GetMap().At(m_mapData.firstBlockPoint).kind = Play::TerrainKind::Floor;

		playerServices().onMoved = {};
	}

	void tutorialScoop(YieldExtended& yield)
	{
		playerServices().onMoved = [&](const Play::CharaVec2& pos, auto)
		{
			if (pos.MapPoint() == m_mapData.sukuuEventPoint)
			{
				playerServices().canMove = false;
			}
		};
		yield.WaitForFalseVal(playerServices().canMove);
		playerServices().onMoved = {};

		// すくう入手イベント発生
		playerServices().overrideCamera = {0, 0};

		auto catSouth = m_play.GetEnemies().Birth(m_play.AsMainContent(), Play::EnSlimeCat());
		catSouth.InitTutorial(m_mapData.catSpawnPoint_South * Play::CellPx_24, Dir4::Up);

		auto catNorth = m_play.GetEnemies().Birth(m_play.AsMainContent(), Play::EnSlimeCat());
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

		auto timescaleController = StartCoro(m_play.AsMainContent(), [&](YieldExtended yield1)
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

		(void)m_bgm.setVolume(0.7);
		waitMessage(yield, U"おや、怪物に囲まれてしまったね", messageWaitMedium);
		waitMessage(yield, U"では、キミに運命を 'スクう' チカラを与えよう", messageWaitMedium);
		(void)m_bgm.setVolume(0.5);

		bool hasScooped{};
		playerServices().canScoop = true;
		playerServices().canScoopTo = [&](const Play::CharaVec2& pos)
		{
			// 左右だけすくえるようにする
			return m_play.GetPlayer().CurrentPos().actualPos.MapPoint().y == pos.MapPoint().y;
		};
		playerServices().onScooped = [&](auto)
		{
			hasScooped = true;
		};

		m_focus.Show(Scene::Center());
		auto scoopingMessage = StartCoro(m_play.AsMainContent(), [&](YieldExtended y)
		{
			waitMessage(y, Gm::IsUsingGamepad()
				               ? U"自分自身をスクってみよう"
				               : U"キミ自身をマウスカーソルでスクってみるといい",
			            messageWaitMedium);
			m_messanger.ShowMessageForever(
				Gm::IsUsingGamepad()
					? U"'RT' を押して壁方向へ向かってみるんだ"
					: U"自分自身を 'ドラッグ' して\n壁の方向へ 'ドロップ' するんだ");
		});
		yield.WaitForTrueVal(hasScooped);
		playerServices().canScoop = false;

		timescaleController.Kill();
		scoopingMessage.Kill();
		m_focus.Hide();
		SetTimeScale(1.0);
		(void)m_bgm.setVolume(1.0);
		yield.WaitForTime(1.0);

		playerServices().overrideCamera = none;

		waitMessage(yield, U"間一髪、一難過ぎ去ったね", messageWaitShort);
		catNorth.Kill();
		catSouth.Kill();
		playerServices().canMove = true;
		playerServices().canScoop = true;
		playerServices().onScooped = {};
		playerServices().canScoopTo = {};
		waitMessage(yield, U"さて、奥へ進もうか", messageWaitShort);
	}

	void tutorialItem(YieldExtended& yield)
	{
		auto knight = m_play.GetEnemies().Birth(m_play.AsMainContent(), Play::EnKnight());
		knight.InitTutorial(m_mapData.knightSpawnPoint * Play::CellPx_24, Dir4::Left);

		bool nearItem{};
		playerServices().onMoved = [&](const Play::CharaVec2& pos, bool isRunning)
		{
			if ((pos.MapPoint() - m_mapData.itemSpawnPoint).manhattanLength() <= 5) nearItem = true;
		};
		const auto leftRegionLimit = [this](const Play::CharaVec2& pos)
		{
			return pos.MapPoint().x < m_mapData.knightBlockPoint.x;
		};
		playerServices().canMoveTo = leftRegionLimit;
		playerServices().canScoopTo = leftRegionLimit;

		yield.WaitForTrueVal(nearItem);
		playerServices().canMove = false;
		waitMessage(yield, U"イイものが落ちているね", messageWaitShortShort);
		playerServices().canMove = true;

		bool obtainedItem{};
		playerServices().onMoved = [&](const Play::CharaVec2& pos, bool isRunning)
		{
			if (pos.MapPoint() != m_mapData.itemSpawnPoint) return;
			obtainedItem = true;
		};

		// アイテム拾うまで待機
		yield.WaitForTrueVal(obtainedItem);
		playerServices().onMoved = {};

		// 閉じ込める
		m_play.GetMap().At(m_mapData.itemBlockPoint).kind = Play::TerrainKind::Wall;

		m_messanger.ShowMessageForever(
			Gm::IsUsingGamepad()
				? U"'L / R' でアイテムを選んで 'A' で使ってみよう"
				: U"アイコンを '左クリック' するか '数字キー' で使ってみよう");

		// 敵をキルするまで待機
		yield.WaitForTrue([&]() { return knight.IsDead(); });

		playerServices().canMoveTo = {};
		playerServices().canScoopTo = {};

		waitMessage(yield, U"うん、いいね", messageWaitShortShort);
	}

	void tutorialFinal(YieldExtended& yield)
	{
		bool nearStairs1{};
		playerServices().onMoved = [&](const Play::CharaVec2& pos, bool isRunning)
		{
			if ((pos.MapPoint() - m_mapData.stairsPoint).manhattanLength() <= 8) nearStairs1 = true;
		};
		yield.WaitForTrueVal(nearStairs1);
		playerServices().onMoved = {};

		playerServices().canMove = false;
		playerServices().canScoop = false;
		waitMessage(yield, U"最後に重要なことを話しておこう", messageWaitShort);
		waitMessage(yield, U"実はこの迷宮でキミが生きられる時間は\n限られている", messageWaitMedium);

		m_play.GetTimeLimiter().SetCountEnabled(true);
		m_focus.Show(Rect(Scene::Size()).tr() + Size{-1, 1} * 144);

		waitMessage(yield, U"砂時計を見てもらいたい", messageWaitShort);

		waitMessage(yield, U"これが0になるとキミは死を迎えるだろう", messageWaitShort);
		waitMessage(yield, U"これは怪物を倒したり、アイテムで回復が可能だ", messageWaitMedium);

		waitMessage(yield, U"では、キミの健闘を祈っているよ", messageWaitShortShort);

		m_focus.Hide();

		playerServices().canMove = true;
		playerServices().canScoop = true;
	}
};

namespace Tutorial
{
	TutorialScene::TutorialScene() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void TutorialScene::Init(bool retrying)
	{
		p_impl->m_retrying = retrying;
		p_impl->Init(*this);
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
