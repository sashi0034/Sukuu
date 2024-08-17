#include "stdafx.h"
#include "TutorialScene.h"

#include "AssetKeys.h"
#include "Assets.generated.h"
#include "Constants.h"
#include "TutorialFocus.h"
#include "TutorialMap.h"
#include "Gm/GameCursor.h"
#include "Gm/GamepadObserver.h"
#include "Gm/LocalizedTextDatabase.h"
#include "Gm/BgmManager.h"
#include "Play/PlayScene.h"
#include "Play/Enemy/EnKnight.h"
#include "Play/Enemy/EnSlimeCat.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlDebugValueWrapper.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"tutorial.scene." + key);
	}

	// constexpr double messageWaitShortShort = 2.0;
	constexpr double messageWaitShort = 3.0;
	constexpr double messageWaitMedium = 4.0;
}

struct Tutorial::TutorialScene::Impl
{
	Play::PlayScene m_playScene{Play::PlayScene::Empty()};
	Play::PlayCore m_play{Play::PlayCore::Empty()};
	TutorialMapData m_mapData{};
	bool m_finished{};
	Play::UiMessenger m_messanger{};
	TutorialFocus m_focus{};
	std::function<void()> m_postDraw{};
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

		m_messanger = m_play.BirthUiMessenger();
		m_focus = m_play.AsUiContent().Birth(TutorialFocus());

		// ポーズ画面にチュートリアル終了ボタンを追加
		m_play.GetPause().AddButtonCancelTutorial([&]()
		{
			Gm::BgmManager::Instance().EndPlay();
			m_finished = true;
		});
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

		// 初回起動時は、Siv3D のロゴを表示させる
		// このときに、ゲームパッドでボタン入力が行われたら、ゲームパッド設定画面に入る
		if (not m_retrying) performLogo(yield);

		startPlayScene();

		performOpening(yield);

		tutorialHowtoMove(yield);

		tutorialScoop(yield);

		tutorialItem(yield);

		playerServices().canMove = true;

		tutorialFinal(yield);

		yield.WaitForTrue([this]() { return m_play.GetPlayer().IsTerminated(); });

		Gm::BgmManager::Instance().EndPlay(3.0);
		yield.WaitForTime(3.0);
		m_finished = true;
	}

	void performLogo(YieldExtended& yield)
	{
#if _DEBUG
		if (GetTomlDebugValueOf<bool>(U"tutorial_from_how_to_move")) return;
#endif

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

			font(Gm::LocalizedText(U"tutorial_start"))
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
#if _DEBUG
		if (GetTomlDebugValueOf<bool>(U"tutorial_from_how_to_move")) return;
#endif

		m_play.GetPause().SetAllowed(false);
		Gm::BgmManager::Instance().RequestPlay({AssetBgms::obake_dance_on_piano, 0.0, 70.0});

		double prologueAlpha{};
		const auto prologueFont = FontAsset(AssetKeys::RocknRoll_Sdf);
		m_postDraw = [&]()
		{
			Rect(Scene::Size()).draw(ColorF{Constants::HardDarkblue});
			prologueFont(Gm::LocalizedText(U"tutorial_open")).drawAt(
				40.0, Scene::Center(), ColorF(1.0, prologueAlpha));
		};
		yield.WaitForExpire(AnimateEasing<EaseInOutSine>(m_play.AsMainContent(), &prologueAlpha, 1.0, 2.0));
		yield.WaitForTime(3.0);
		yield.WaitForExpire(AnimateEasing<EaseInOutSine>(m_play.AsMainContent(), &prologueAlpha, 0.0, 2.0));

		// プレイヤーのカメラ移動
		m_play.GetPlayer().PerformCinematicOpening(8.0);

		// 真っ黒な画面から徐々に明るくしていく
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
		waitMessage(yield, Gm::LocalizedText(U"tutorial_welcome"), messageWaitMedium);
		waitMessage(yield, Gm::LocalizedText(U"tutorial_lets_start_description"), messageWaitMedium);
		playerServices().canMove = true;
		int movedCount{};
		int runningCount{};
		playerServices().onMoved = [&](auto, bool isRunning)
		{
			if (isRunning) runningCount++;
			movedCount++;
		};
		m_messanger.ShowMessageForever(
			fmt::format(
				Gm::LocalizedText(U"tutorial_how_to_move").data(),
				Gm::LocalizedText(Gm::IsUsingGamepad() ? U"button_d_pad" : U"button_wasd_or_arrow").data()));

		yield.WaitForTrue([&]
		{
			return movedCount > 10;
		});
		m_messanger.ShowMessageForever(
			fmt::format(
				Gm::LocalizedText(U"tutorial_how_to_run").data(),
				Gm::IsUsingGamepad() ? U"'B'" : U"'Shift'"));

		yield.WaitForTrue([&]
		{
			return runningCount > 10;
		});
		waitMessage(yield, Gm::LocalizedText(U"tutorial_go_depth"), messageWaitMedium);

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

		Gm::BgmManager::Instance().OverrideVolumeRate(0.7);
		waitMessage(yield, Gm::LocalizedText(U"tutorial_surrounded_monsters"), messageWaitMedium);
		waitMessage(yield, Gm::LocalizedText(U"tutorial_introduce_sukuu"), messageWaitMedium);
		Gm::BgmManager::Instance().OverrideVolumeRate(0.5);

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
		m_messanger.ShowMessageForever(
			Gm::LocalizedText(Gm::IsUsingGamepad() ? U"tutorial_sukuu_by_gp" : U"tutorial_sukuu_by_km"));
		yield.WaitForTrueVal(hasScooped);
		playerServices().canScoop = false;

		timescaleController.Kill();
		m_focus.Hide();
		SetTimeScale(1.0);
		Gm::BgmManager::Instance().OverrideVolumeRate(none);
		yield.WaitForTime(1.0);

		playerServices().overrideCamera = none;

		waitMessage(yield, Gm::LocalizedText(U"tutorial_succeed_sukuu"), messageWaitShort);
		catNorth.Kill();
		catSouth.Kill();
		playerServices().canMove = true;
		playerServices().canScoop = true;
		playerServices().onScooped = {};
		playerServices().canScoopTo = {};
		waitMessage(yield, Gm::LocalizedText(U"tutorial_go_depth"), messageWaitShort);
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
		waitMessage(yield, Gm::LocalizedText(U"tutorial_introduce_item"), messageWaitMedium);
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
			Gm::LocalizedText(Gm::IsUsingGamepad() ? U"tutorial_use_item_by_gp" : U"tutorial_use_item_by_km"));

		// 敵をキルするまで待機
		yield.WaitForTrue([&]() { return knight.IsDead(); });

		playerServices().canMoveTo = {};
		playerServices().canScoopTo = {};

		waitMessage(yield, Gm::LocalizedText(U"tutorial_kill_enemy"), messageWaitShort);
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
		waitMessage(yield, Gm::LocalizedText(U"tutorial_introduce_limit"), messageWaitShort);
		waitMessage(yield, Gm::LocalizedText(U"tutorial_whats_limit"), messageWaitMedium);

		m_play.GetTimeLimiter().SetCountEnabled(true);
		m_focus.Show(Rect(Scene::Size()).tr() + Size{-1, 1} * 144);

		waitMessage(yield, Gm::LocalizedText(U"tutorial_see_clock"), messageWaitShort);

		waitMessage(yield, Gm::LocalizedText(U"tutorial_when_time_up"), messageWaitShort);
		waitMessage(yield, Gm::LocalizedText(U"tutorial_heal_time"), messageWaitMedium);

		waitMessage(yield, Gm::LocalizedText(U"tutorial_finish"), messageWaitShort);

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
