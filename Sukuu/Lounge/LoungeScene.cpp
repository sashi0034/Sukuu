#include "stdafx.h"
#include "LoungeScene.h"

#include "LoungeMap.h"
#include "Play/PlayScene.h"
#include "Util/EasingAnimation.h"

namespace
{
	using namespace Lounge;
}

struct LoungeScene::Impl
{
	Play::PlayScene m_playScene{Play::PlayScene::Empty()};
	Play::PlayCore m_play{Play::PlayCore::Empty()};
	LoungeMapData m_mapData{};

	// PlayScene などのあとに描画する
	std::function<void()> m_postDraw{};

	void Init(ActorView self, const LoungeEnterArgs& args)
	{
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

		// PlayScene 初期化
		m_playScene.Init({
			.designatedMap = Play::DesignatedMapInfo{
				.map = m_mapData.map,
				.initialPlayerPos = m_mapData.initialPlayerPoint * Play::CellPx_24
			},
			.playerPersonal = {},
			.timeLimiter = Play::TimeLimiterData{
				.maxTime = 90, // TODO: プレイヤーの最後の maxTime から取得したいかも
				.remainingTime = 90
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
		timeLimiter.MisrepresentedAsIfZero();
	}

	void flowchartLoop(YieldExtended& yield)
	{
		yield();

		// プレイヤーのカメラ移動
		m_play.GetPlayer().PerformTutorialOpening(1.0);

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
	}

	bool LoungeScene::IsConcluded()
	{
		return p_impl->m_play.GetPlayer().IsTerminated();
	}
}
