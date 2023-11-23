#include "stdafx.h"
#include "PlayScene.h"

#include "Player.h"
#include "Gimmick/GimmickInstaller.h"
#include "Map/BgMapDrawer.h"
#include "Map/MapGrid.h"
#include "Other/CaveSnowfall.h"
#include "Other/CaveVision.h"
#include "Other\FloorLevelDesign.h"
#include "Other/PlayingPause.h"
#include "UI/UiCurrentFloor.h"
#include "UI/UiDashKeep.h"
#include "UI/UiFloorTransition.h"
#include "UI/UiGameOver.h"
#include "UI/UiItemContainer.h"
#include "UI/UiMiniMap.h"
#include "UI/UiTimeLimiter.h"
#include "Util/ActorContainer.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	namespace CameraKind
	{
		enum Value
		{
			Player,
			Debug,
		};
	}

	template <typename T>
	T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.scene." + key);
	}

	using namespace Play;
	PlayScene s_instance{PlayScene::Empty()};
}

class Play::PlayScene::Impl
{
public:
	MapGrid m_map{};
	GimmickGrid m_gimmick{};
	BgMapDrawer m_bgMapDrawer{};
	Player m_player{};
	EnemyContainer m_enemies{};
	Camera2D m_debugCamera{};
	CameraKind::Value m_camera = CameraKind::Player;
	ActorContainer m_ui{};
	UiItemContainer m_uiItemContainer{};
	UiMiniMap m_uiMiniMap{};
	UiTimeLimiter m_uiTimeLimiter{};
	UiDashKeep m_uiDashKeep{};
	CaveVision m_caveVision{};
	int m_hitStoppingRequested{};
	EffectWrapper m_fgEffect{};
	EffectWrapper m_bgEffect{};
	ITutorialSetting* m_tutorial{};
	UiFloorTransition m_floorTransition{};
	UiCurrentFloor m_uiCurrentFloor{};
	UiGameOver m_uiGameOver{};
	int m_floorIndex{};
	MeasuredSecondsArray m_measuredSeconds{};
	PlayingPause m_pause{};

	void Init(ActorView self, const PlaySingletonData& data)
	{
		m_tutorial = data.tutorial;
		m_floorIndex = data.floorIndex;
		m_measuredSeconds = data.measuredSeconds;

		if (const auto tutorial = data.tutorial)
		{
			m_map = tutorial->GetMap();
		}
		else
		{
			m_map = GenerateFloorMap(data.floorIndex);
		}

		m_gimmick.resize(m_map.Data().size(), GimmickKind::None);

		// 生成
		m_fgEffect = self.AsParent().Birth(EffectWrapper(FgEffectPriority));
		m_bgEffect = self.AsParent().Birth(EffectWrapper(BgEffectPriority));

		m_fgEffect.GetEffect().setMaxLifeTime(std::numeric_limits<double>::max());
		m_fgEffect.GetEffect().add([this](auto)
		{
			m_bgMapDrawer.PostDraw();
			return true;
		});
		m_bgMapDrawer.SetBgShader(GetFloorBgShader(data.floorIndex));

		m_player = self.AsParent().Birth(Player());

		if (IsFloorSnowfall(data.floorIndex)) self.AsParent().Birth(CaveSnowfall());

		m_uiItemContainer = m_ui.Birth(UiItemContainer());

		m_uiMiniMap = m_ui.Birth(UiMiniMap());

		m_uiTimeLimiter = m_ui.Birth(UiTimeLimiter());

		m_uiDashKeep = m_ui.Birth(UiDashKeep());

		m_uiCurrentFloor = m_ui.Birth(UiCurrentFloor());

		m_uiGameOver = m_ui.Birth(UiGameOver());

		ensureInitializedTransition();

		// 初期化
		if (const auto tutorial = data.tutorial)
		{
			m_player.Init(data.playerPersonal, tutorial->InitialPlayerPos());
		}
		else
		{
			GenerateEnemiesAndGimmicks(data.floorIndex, m_map, self, m_enemies, m_gimmick);
			m_player.Init(data.playerPersonal, GetInitialPos(m_map, m_map.Category() == MapCategory::Maze));

			m_uiCurrentFloor.Init(data.floorIndex);
			m_uiGameOver.Init(data.floorIndex);

			m_pause.SetAllowed(true);
		}

		m_uiMiniMap.Init(m_map.Data().size());

		m_uiTimeLimiter.Init(data.timeLimiter);

		m_uiDashKeep.Init(data.dashKeeping);
	}

	ActorWeak StartTransition(int floorIndex)
	{
		ensureInitializedTransition();
		return m_floorTransition.PerformOpen(floorIndex);
	}

	void UpdateScene(PlayScene& self)
	{
#ifdef  _DEBUG
		// デバッグ用にカメラ変更
		if (KeyNum0.down())
		{
			m_camera = m_camera != CameraKind::Debug ? CameraKind::Debug : CameraKind::Player;
		}
#endif
		if (m_camera == CameraKind::Debug) m_debugCamera.update();

		const auto t = m_camera == CameraKind::Player
			               ? Transformer2D(m_player.CameraTransform(), TransformCursor::Yes)
			               : m_debugCamera.createTransformer();

		const ScopedRenderStates2D sampler{SamplerState::BorderNearest};

		// 背景描画
		m_bgMapDrawer.UpdateDraw(self);

		// ポーズ中なら更新無し
		m_pause.Update();
		if (m_pause.IsPaused()) return;

		// キャラクターなどの通常更新
		self.ActorBase::Update();
		m_enemies.Refresh();
		m_bgEffect.GetEffect().setSpeed(GetTimeScale());
		m_fgEffect.GetEffect().setSpeed(GetTimeScale());

		// 視界マスク更新
		m_caveVision.UpdateMask(m_player.CurrentPos().viewPos + Point(CellPx_24, CellPx_24) / 2);

		m_measuredSeconds[m_floorIndex] += GetDeltaTime();

		// ヒットストッピング管理
		SetTimeScale(m_hitStoppingRequested > 0 ? getToml<double>(U"hitstopping_timescale") : 1.0);
	}

private:
	void ensureInitializedTransition()
	{
		if (m_floorTransition.IsInitialized()) return;
		m_floorTransition = m_ui.Birth(UiFloorTransition());
		m_floorTransition.Init();
	}
};

namespace Play
{
	PlayScene::PlayScene() = default;

	PlayScene PlayScene::Empty()
	{
		return {};
	}

	PlayScene PlayScene::Create()
	{
		PlayScene p{};
		p.p_impl = std::make_shared<Impl>();
		s_instance = p;
		return p;
	}

	void PlayScene::Init(const PlaySingletonData& data)
	{
		p_impl->Init(*this, data);
	}

	void PlayScene::Update()
	{
		p_impl->UpdateScene(*this);
		p_impl->m_caveVision.UpdateScreen();
		if (not p_impl->m_pause.IsPaused()) p_impl->m_ui.Update();
	}

	ActorWeak PlayScene::StartTransition(int floorIndex)
	{
		return p_impl->StartTransition(floorIndex);
	}

	ActorWeak PlayScene::EndTransition()
	{
		p_impl->m_pause.SetAllowed(false);
		return p_impl->m_floorTransition.PerformClose();
	}

	ActorWeak PlayScene::PerformGameOver()
	{
		p_impl->m_pause.SetAllowed(false);
		return p_impl->m_uiGameOver.StartPerform();
	}

	PlayScene& PlayScene::Instance()
	{
		return s_instance;
	}

	MapGrid& PlayScene::GetMap()
	{
		return p_impl->m_map;
	}

	const MapGrid& PlayScene::GetMap() const
	{
		return p_impl->m_map;
	}

	GimmickGrid& PlayScene::GetGimmick()
	{
		return p_impl->m_gimmick;
	}

	const GimmickGrid& PlayScene::GetGimmick() const
	{
		return p_impl->m_gimmick;
	}

	Player& PlayScene::GetPlayer()
	{
		return p_impl->m_player;
	}

	const Player& PlayScene::GetPlayer() const
	{
		return p_impl->m_player;
	}

	EnemyContainer& PlayScene::GetEnemies()
	{
		return p_impl->m_enemies;
	}

	const EnemyContainer& PlayScene::GetEnemies() const
	{
		return p_impl->m_enemies;
	}

	UiTimeLimiter& PlayScene::GetTimeLimiter()
	{
		return p_impl->m_uiTimeLimiter;
	}

	const UiTimeLimiter& PlayScene::GetTimeLimiter() const
	{
		return p_impl->m_uiTimeLimiter;
	}

	UiMiniMap& PlayScene::GetMiniMap()
	{
		return p_impl->m_uiMiniMap;
	}

	const UiMiniMap& PlayScene::GetMiniMap() const
	{
		return p_impl->m_uiMiniMap;
	}

	UiDashKeep& PlayScene::GetDashKeep()
	{
		return p_impl->m_uiDashKeep;
	}

	const UiDashKeep& PlayScene::GetDashKeep() const
	{
		return p_impl->m_uiDashKeep;
	}

	EffectWrapper& PlayScene::FgEffect()
	{
		return p_impl->m_fgEffect;
	}

	const EffectWrapper& PlayScene::FgEffect() const
	{
		return p_impl->m_fgEffect;
	}

	EffectWrapper& PlayScene::BgEffect()
	{
		return p_impl->m_bgEffect;
	}

	const EffectWrapper& PlayScene::BgEffect() const
	{
		return p_impl->m_bgEffect;
	}

	ITutorialSetting* PlayScene::Tutorial()
	{
		return p_impl->m_tutorial;
	}

	const ITutorialSetting* PlayScene::Tutorial() const
	{
		return p_impl->m_tutorial;
	}

	void PlayScene::RequestHitstopping(double time)
	{
		p_impl->m_hitStoppingRequested++;
		StartCoro(*this, [this, time](YieldExtended yield)
		{
			yield.WaitForTime(time, Scene::DeltaTime);
			p_impl->m_hitStoppingRequested--;
		});
	}

	PlaySingletonData PlayScene::CopyData() const
	{
		return PlaySingletonData{
			.floorIndex = p_impl->m_floorIndex,
			.measuredSeconds = p_impl->m_measuredSeconds,
			.playerPersonal = p_impl->m_player.PersonalData(),
			.timeLimiter = p_impl->m_uiTimeLimiter.GetData(),
			.dashKeeping = p_impl->m_uiDashKeep.IsKeeping(),
		};
	}
}
