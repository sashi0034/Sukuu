#include "stdafx.h"
#include "PlayCore.h"

#include "AssetKeys.h"
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
		return Util::GetTomlParameter<T>(U"play.core." + key);
	}

	using namespace Play;

	PlayCore* s_instance{};

	struct GradientBlueCb
	{
		Float2 center;
	};

	struct SlowMotionState
	{
		RenderTexture bufferTexture{};
		double time{};
	};
}

class Play::PlayCore::Impl
{
public:
	ActorContainer m_main{};
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
	SlowMotionState m_slowMotion{};

	void Init(const PlaySingletonData& data)
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
		m_fgEffect = m_main.Birth(EffectWrapper(FgEffectPriority));
		m_bgEffect = m_main.Birth(EffectWrapper(BgEffectPriority));

		m_fgEffect.GetEffect().setMaxLifeTime(std::numeric_limits<double>::max());
		m_fgEffect.GetEffect().add([this](auto)
		{
			m_bgMapDrawer.PostDraw();
			return true;
		});
		m_bgMapDrawer.SetBgShader(GetFloorBgShader(data.floorIndex));

		m_player = m_main.Birth(Player());

		if (IsFloorSnowfall(data.floorIndex)) m_main.Birth(CaveSnowfall());

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
			GenerateEnemiesAndGimmicks(data.floorIndex, m_map, m_main, m_enemies, m_gimmick);
			m_player.Init(data.playerPersonal, GetInitialPos(m_map, m_map.Category() == MapCategory::Maze));

			m_uiCurrentFloor.Init(data.floorIndex);
			m_uiGameOver.Init(data.floorIndex);

			m_pause.SetAllowed(true);
		}

		m_uiItemContainer.Init(data.itemIndexing);

		m_uiMiniMap.Init(m_map.Data().size());

		m_uiTimeLimiter.Init(data.timeLimiter);

		m_uiDashKeep.Init(data.dashKeeping);

		m_slowMotion.bufferTexture = RenderTexture(Scene::Size());
	}

	ActorWeak StartTransition(int floorIndex)
	{
		ensureInitializedTransition();
		return m_floorTransition.PerformOpen(floorIndex);
	}

	void UpdateScene()
	{
#ifdef  _DEBUG
		// デバッグ用にカメラ変更
		if (KeyNum0.down())
		{
			m_camera = m_camera != CameraKind::Debug ? CameraKind::Debug : CameraKind::Player;
		}
#endif
		if (m_camera == CameraKind::Debug) m_debugCamera.update();

		const auto transformer = m_camera == CameraKind::Player
			                         ? Transformer2D(m_player.CameraTransform(), TransformCursor::Yes)
			                         : m_debugCamera.createTransformer();

		const ScopedRenderStates2D sampler{SamplerState::BorderNearest};

		const bool isSlowMotion = not m_pause.IsPaused() && m_player.IsSlowMotion();
		const ScopedRenderTarget2D rt0{
			isSlowMotion ? ScopedRenderTarget2D(m_slowMotion.bufferTexture) : ScopedRenderTarget2D(none)
		};
		m_slowMotion.time = isSlowMotion ? m_slowMotion.time + Scene::DeltaTime() : 0;

		// 背景描画
		m_bgMapDrawer.UpdateDraw();

		// ポーズ中なら更新無し
		m_pause.Update();
		if (m_pause.IsPaused()) return;

		// キャラクターなどの通常更新
		m_main.Update();
		m_enemies.Refresh();
		m_bgEffect.GetEffect().setSpeed(GetTimeScale());
		m_fgEffect.GetEffect().setSpeed(GetTimeScale());

		// 視界マスク更新
		m_caveVision.UpdateMask(m_player.CurrentPos().viewPos + Point(CellPx_24, CellPx_24) / 2);

		// 実時間でフロア経過時間を計測
		m_measuredSeconds[m_floorIndex] += Scene::DeltaTime();

		// ヒットストッピング管理
		SetTimeScale(m_hitStoppingRequested > 0
			             ? getToml<double>(U"hitstopping_timescale")
			             : (isSlowMotion
				                ? getToml<double>(U"slow_timescale")
				                : 1.0)
		);

		if (isSlowMotion)
		{
			// スローモーションのときの描画
			const auto playerScreenPos = Graphics2D::GetLocalTransform().transformPoint(
				m_player.CurrentPos().actualPos.movedBy(Vec2::One() * CellPx_24 / 2)) / Scene::Size();
			ConstantBuffer<GradientBlueCb> cb{};
			cb->center = playerScreenPos;
			Graphics2D::SetPSConstantBuffer(1, cb);

			const Transformer2D rollbackTransform{Mat3x2::Identity(), Transformer2D::Target::SetLocal};
			const ScopedCustomShader2D slowMotionShader{PixelShaderAsset(AssetKeys::PsGradientBlur)};
			const ScopedRenderTarget2D rt1{none};
			const ScopedRenderStates2D rs1{SamplerState::ClampLinear};
			(void)m_slowMotion.bufferTexture.draw();
		}
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
	PlayCore::PlayCore() : p_impl(std::make_shared<Impl>())
	{
		if (s_instance != nullptr) System::MessageBoxOK(U"Duplicated PlayCore instance", MessageBoxStyle::Error);
		else s_instance = this;
	}

	PlayCore::~PlayCore()
	{
		if (s_instance == this) s_instance = nullptr;
	}

	ActorContainer& PlayCore::AsMainContent()
	{
		return p_impl->m_main;
	}

	void PlayOperationCore::Init(const PlaySingletonData& data)
	{
		p_impl->Init(data);
	}

	void PlayOperationCore::Update()
	{
		p_impl->UpdateScene();
		p_impl->m_caveVision.UpdateScreen();
		if (not p_impl->m_pause.IsPaused()) p_impl->m_ui.Update();
	}

	ActorWeak PlayCore::StartTransition(int floorIndex)
	{
		return p_impl->StartTransition(floorIndex);
	}

	ActorWeak PlayCore::EndTransition()
	{
		p_impl->m_pause.SetAllowed(false);
		return p_impl->m_floorTransition.PerformClose();
	}

	ActorWeak PlayCore::PerformGameOver()
	{
		p_impl->m_pause.SetAllowed(false);
		return p_impl->m_uiGameOver.StartPerform();
	}

	PlayCore& PlayCore::Instance()
	{
		return *s_instance;
	}

	MapGrid& PlayCore::GetMap()
	{
		return p_impl->m_map;
	}

	const MapGrid& PlayCore::GetMap() const
	{
		return p_impl->m_map;
	}

	GimmickGrid& PlayCore::GetGimmick()
	{
		return p_impl->m_gimmick;
	}

	const GimmickGrid& PlayCore::GetGimmick() const
	{
		return p_impl->m_gimmick;
	}

	Player& PlayCore::GetPlayer()
	{
		return p_impl->m_player;
	}

	const Player& PlayCore::GetPlayer() const
	{
		return p_impl->m_player;
	}

	EnemyContainer& PlayCore::GetEnemies()
	{
		return p_impl->m_enemies;
	}

	const EnemyContainer& PlayCore::GetEnemies() const
	{
		return p_impl->m_enemies;
	}

	UiTimeLimiter& PlayCore::GetTimeLimiter()
	{
		return p_impl->m_uiTimeLimiter;
	}

	const UiTimeLimiter& PlayCore::GetTimeLimiter() const
	{
		return p_impl->m_uiTimeLimiter;
	}

	UiMiniMap& PlayCore::GetMiniMap()
	{
		return p_impl->m_uiMiniMap;
	}

	const UiMiniMap& PlayCore::GetMiniMap() const
	{
		return p_impl->m_uiMiniMap;
	}

	UiDashKeep& PlayCore::GetDashKeep()
	{
		return p_impl->m_uiDashKeep;
	}

	const UiDashKeep& PlayCore::GetDashKeep() const
	{
		return p_impl->m_uiDashKeep;
	}

	EffectWrapper& PlayCore::FgEffect()
	{
		return p_impl->m_fgEffect;
	}

	const EffectWrapper& PlayCore::FgEffect() const
	{
		return p_impl->m_fgEffect;
	}

	EffectWrapper& PlayCore::BgEffect()
	{
		return p_impl->m_bgEffect;
	}

	const EffectWrapper& PlayCore::BgEffect() const
	{
		return p_impl->m_bgEffect;
	}

	ITutorialSetting* PlayCore::Tutorial()
	{
		return p_impl->m_tutorial;
	}

	const ITutorialSetting* PlayCore::Tutorial() const
	{
		return p_impl->m_tutorial;
	}

	void PlayCore::RequestHitstopping(double time)
	{
		p_impl->m_hitStoppingRequested++;
		StartCoro(p_impl->m_main, [impl = p_impl.get(), time](YieldExtended yield)
		{
			yield.WaitForTime(time, Scene::DeltaTime);
			impl->m_hitStoppingRequested--;
		});
	}

	PlaySingletonData PlayCore::CopyData() const
	{
		return PlaySingletonData{
			.floorIndex = p_impl->m_floorIndex,
			.measuredSeconds = p_impl->m_measuredSeconds,
			.playerPersonal = p_impl->m_player.PersonalData(),
			.timeLimiter = p_impl->m_uiTimeLimiter.GetData(),
			.dashKeeping = p_impl->m_uiDashKeep.IsKeeping(),
			.itemIndexing = p_impl->m_uiItemContainer.GetIndexing()
		};
	}
}
