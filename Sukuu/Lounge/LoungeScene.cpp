#include "stdafx.h"
#include "LoungeScene.h"

#include "Assets.generated.h"
#include "LoungeBgDrawer.h"
#include "LoungeMap.h"
#include "Gm/BgmManager.h"
#include "Gm/LocalizedTextDatabase.h"
#include "Play/PlayingUra.h"
#include "Play/PlayScene.h"
#include "Play/Map/BgMapDrawer.h"
#include "Play/Other/PlayingTips.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlDebugValueWrapper.h"
#include "Util/VisualStudioHotReloadDetectorAddon.h"

namespace
{
	using namespace Lounge;

	int s_enteredCount{};
	double s_bonusAppearedTimestamp{};

	constexpr int floorAllowedToContinueFromMiddle_10 = 10;

	enum class LoungeStairs
	{
		ToTitle,
		ToContinueFromBeginning,
		ToContinueFromMiddle,
	};

	void installBonusItemsCrisscross(Play::GimmickGrid& gimmick, const LoungeMapData& mapData, Play::GimmickKind item)
	{
		gimmick[mapData.bookPoint.movedBy(-1, -1)] = item;
		gimmick[mapData.bookPoint.movedBy(1, -1)] = item;
		gimmick[mapData.bookPoint.movedBy(-1, 1)] = item;
		gimmick[mapData.bookPoint.movedBy(1, 1)] = item;
	}

	void installBonusItemsAbundantly(Play::GimmickGrid& gimmick, const LoungeMapData& mapData, Play::GimmickKind item)
	{
		for (int x = -1; x <= 1; ++x)
		{
			for (int y = -1; y <= 1; ++y)
			{
				if (x == 0 && y == 0) continue;
				gimmick[mapData.bookPoint.movedBy(x, y)] = item;
			}
		}
	}

	void setupBonusItems(Play::GimmickGrid& gimmick, const LoungeMapData& mapData)
	{
		if (Scene::Time() < 300 + s_bonusAppearedTimestamp)
		{
			// 一定時間感覚を空けないとボーナスは出現しない
			return;
		}

		// if (s_enteredCount < 3) return;

		if (RandomBool(0.3))
		{
			installBonusItemsAbundantly(gimmick, mapData, Play::GimmickKind::Item_Pin);
			s_bonusAppearedTimestamp = Scene::Time();
		}
		else if (RandomBool(0.1))
		{
			installBonusItemsCrisscross(gimmick, mapData, Play::GimmickKind::Item_Wing);
			s_bonusAppearedTimestamp = Scene::Time();
		}
	}

	void setupGimmicks(Play::GimmickGrid& gimmick, const LoungeMapData& mapData)
	{
		gimmick[mapData.stairsToTitlePoint] = Play::GimmickKind::Stairs;
		gimmick[mapData.stairsToContinueFromBeginningPoint] = Play::GimmickKind::Stairs;
		gimmick[mapData.stairsToContinueFromMiddlePoint] = Play::GimmickKind::Stairs;

		gimmick[mapData.bookPoint] = Play::GimmickKind::SemiItem_Book;

		if (not Play::IsPlayingUra())
		{
			// 稀にボーナスアイテム設置
			setupBonusItems(gimmick, mapData);
		}
	}
}

struct LoungeScene::Impl
{
	LoungeEnterArgs m_args{};
	Play::PlayScene m_playScene{Play::PlayScene::Empty()};
	Play::PlayCore m_play{Play::PlayCore::Empty()};
	Play::UiMessenger m_messenger{};
	LoungeMapData m_mapData{};
	LoungeBgDrawer m_bgDrawer{};

	// 途中からコンティニューする際のフロア
	int m_floorForContinueFromMiddle{};

	// 階段に乗ったときに移動する次の階層
	int m_nextFloor{};

	// PlayScene などのあとに描画する
	std::function<void()> m_postDraw{};

	struct
	{
		bool hasBook{};
		LoungeStairs steppedStairs{};
	} m_event;

	void Init(ActorView self, const LoungeEnterArgs& args)
	{
		s_enteredCount++;

		m_args = args;
		m_playScene = self.AsParent().Birth(Play::PlayScene::Create());
		m_play = m_playScene.GetCore();
		startPlayScene();

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
		m_mapData = GetLoungeMap();

		if (m_args.reachedFloor < floorAllowedToContinueFromMiddle_10)
		{
			// 第 10 層未満の場合は中間地点から始められない
			if (m_args.reachedFloor >= floorAllowedToContinueFromMiddle_10 / 2 && not Play::IsPlayingUra())
			{
				// しかし、第 5 層以上であれば第 5 層から始められることとする
				m_floorForContinueFromMiddle = floorAllowedToContinueFromMiddle_10 / 2;
				m_bgDrawer.SetContinueFromMiddle(m_floorForContinueFromMiddle);
			}
			else
			{
				// 橋を削除
				m_mapData.RemoveBridgeEntranceForMiddle();
			}
		}
		else
		{
			// 第 10 層以上の場合はその半分の地点から始められる
			m_floorForContinueFromMiddle = (m_args.reachedFloor + 1) / 2;
			m_bgDrawer.SetContinueFromMiddle(m_floorForContinueFromMiddle);
		}

		// PlayScene 初期化
		m_playScene.Init({
			.designatedMap = Play::DesignatedMapInfo{
				.map = m_mapData.map,
				.initialPlayerPos = m_mapData.initialPlayerPoint * Play::CellPx_24
			},
			.playerPersonal = {},
			.timeLimiter = Play::TimeLimiterData{
				.maxTime = 60, // TODO: プレイヤーの最後の maxTime から取得したいかも?
				.remainingTime = 60
			}
		});

		// ギミック反映
		setupGimmicks(m_play.GetGimmick(), m_mapData);

		m_messenger = m_play.BirthUiMessenger();

		// プレイヤーの設定
		playerServices().forcedImmortal = true;
		playerServices().canMove = false;
		playerServices().canScoop = false;
		playerServices().onMoved = [this](const Vec2& pos, bool) { onPlayerMoved(pos); };

		// タイムリミットの設定
		auto& timeLimiter = m_play.GetTimeLimiter();
		timeLimiter.SetCountEnabled(false);
		timeLimiter.SetImmortal(true);
		timeLimiter.MisrepresentedAsIfZero();

		// 背景描画の設定
		m_play.SetBgCustomDrawer(Play::BgCustomDrawer{
			.backDrawer = [&](Rect r) { m_bgDrawer.DrawBack(m_mapData, r); },
			.frontDrawer = [&]() { m_bgDrawer.DrawFront(m_mapData); }
		});

		m_play.EnableCaveVision(false);

		m_play.GetPause().AddButtonExitGame();
	}

	void onPlayerMoved(Vec2 pos)
	{
		const auto playerPoint = m_play.GetPlayer().CurrentPoint();
		if (playerPoint == m_mapData.bookPoint && not m_event.hasBook)
		{
			// TIPS を表示
			m_event.hasBook = true;
			m_messenger.ShowMessage(Gm::LocalizedText(Play::GetPlayingTips(m_args.reachedFloor)), 8.0);
		}
		else if (playerPoint == m_mapData.stairsToTitlePoint)
		{
			m_event.steppedStairs = LoungeStairs::ToTitle;
		}
		else if (playerPoint == m_mapData.stairsToContinueFromBeginningPoint)
		{
			m_event.steppedStairs = LoungeStairs::ToContinueFromBeginning;
			m_nextFloor = 1;
		}
		else if (playerPoint == m_mapData.stairsToContinueFromMiddlePoint)
		{
			m_event.steppedStairs = LoungeStairs::ToContinueFromMiddle;
			m_nextFloor = m_floorForContinueFromMiddle;
		}
	}

	void flowchartLoop(YieldExtended& yield)
	{
		yield();

		// プレイヤーのカメラ移動
		m_play.GetPlayer().PerformCinematicOpening(1.0);

		// 真っ黒な画面から徐々に明るくしていく
		double rate = 1.0;
		m_postDraw = [&]()
		{
			Rect(Scene::Size()).draw(ColorF{Constants::HardDarkblue, rate});
		};
		yield.WaitForExpire(
			AnimateEasing<EaseInQuad>(m_play.AsMainContent(), &rate, 0.0, 1.0));
		m_postDraw = {};

		// 行動可能にする
		m_play.GetPause().SetAllowed(true);
		playerServices().canMove = true;

		// BGM
		Gm::BgmManager::Instance().RequestPlay({AssetBgms::zzzzzz, 10.0000, 60.5000});

#if _DEBUG
		if (GetTomlDebugValueOf<bool>(U"tips_test"))
		{
			// TIPS テスト表示 (Space で次の TIPS へ)
			const auto tips = Play::GetAllPlayingTips();
			for (const auto& t : tips)
			{
				yield();
				m_messenger.ShowMessage(Gm::LocalizedText(t), 5.0);
				yield.WaitForTrue([]() { return KeySpace.down(); });
			}
		}
#endif
	}
};

namespace Lounge
{
	LoungeScene::LoungeScene() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void LoungeScene::Init(const LoungeEnterArgs& args)
	{
		p_impl->Init(*this, args);
	}

	void LoungeScene::Update()
	{
		ActorBase::Update();
		if (p_impl->m_postDraw) p_impl->m_postDraw();

#if _DEBUG
		if (IsVisualStudioHotReloaded())
		{
			p_impl->m_mapData = GetLoungeMap();
		}
#endif
	}

	bool LoungeScene::IsConcluded()
	{
		return p_impl->m_play.GetPlayer().IsTerminated();
	}

	bool LoungeScene::IsReturnToTitle() const
	{
		return p_impl->m_event.steppedStairs == LoungeStairs::ToTitle;
	}

	int LoungeScene::NextFloor() const
	{
		return p_impl->m_nextFloor;
	}

	Play::PlaySingletonData LoungeScene::GetPlayData() const
	{
		return p_impl->m_play.CopyData();
	}
}
