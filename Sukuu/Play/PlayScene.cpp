#include "stdafx.h"
#include "PlayScene.h"

#include "Player.h"
#include "Enemy/EnKnight.h"
#include "Enemy/EnSlimeCat.h"
#include "Gimmick/GimmickInstaller.h"
#include "Map/BgMapDrawer.h"
#include "Map/DungeonGenerator.h"
#include "Map/MapGrid.h"
#include "Map/MazeGenerator.h"
#include "Other/CaveVision.h"
#include "UI/UiFloorTransition.h"
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
			Max,
		};
	}

	template <typename T>
	T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.scene." + key);
	}
}

class Play::PlayScene::Impl
{
public:
	MapGrid m_map;
	GimmickGrid m_gimmick;
	BgMapDrawer m_bgMapDrawer{};
	Player m_player;
	EnemyContainer m_enemies{};
	Camera2D m_debugCamera;
	CameraKind::Value m_camera = CameraKind::Player;
	ActorContainer m_ui{};
	UiItemContainer m_uiItemContainer;
	UiMiniMap m_uiMiniMap;
	UiTimeLimiter m_uiTimeLimiter;
	CaveVision m_caveVision{};
	int m_hitStoppingRequested{};
	EffectWrapper m_fgEffect{};
	EffectWrapper m_bgEffect{};
	ITutorialSetting* m_tutorial;
	UiFloorTransition m_floorTransition{};

	void Init(ActorView self, const PlaySingletonData& data)
	{
		m_tutorial = data.tutorial;

		if (const auto tutorial = data.tutorial)
		{
			m_map = tutorial->GetMap();
		}
		else
		{
			m_map = generateMap();
		}

		m_gimmick.resize(m_map.Data().size(), GimmickKind::None);

		// 生成
		m_fgEffect = self.AsParent().Birth(EffectWrapper(32767));
		m_bgEffect = self.AsParent().Birth(EffectWrapper(-32768));

		m_player = self.AsParent().Birth(Player());

		m_uiItemContainer = m_ui.Birth(UiItemContainer());

		m_uiMiniMap = m_ui.Birth(UiMiniMap());

		m_uiTimeLimiter = m_ui.Birth(UiTimeLimiter());

		ensureInitializedTransition();

		// 初期化
		if (const auto tutorial = data.tutorial)
		{
			m_player.Init(data.playerPersonal, tutorial->InitialPlayerPos());
		}
		else
		{
			InstallGimmicks(m_gimmick, m_map);
			installEnemies(self);
			m_player.Init(data.playerPersonal, GetInitialPos(m_map));
		}

		m_uiMiniMap.Init(m_map.Data().size());

		m_uiTimeLimiter.Init(data.timeLimiter);
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
			m_camera =
				static_cast<CameraKind::Value>((m_camera + 1) % CameraKind::Max);
#endif
		if (m_camera == CameraKind::Debug) m_debugCamera.update();

		const auto t = m_camera == CameraKind::Player
			               ? Transformer2D(m_player.CameraTransform(), TransformCursor::Yes)
			               : m_debugCamera.createTransformer();

		const ScopedRenderStates2D sampler{SamplerState::BorderNearest};

		// 背景描画
		m_bgMapDrawer.Tick(self);

		// キャラクターなどの通常更新
		self.ActorBase::Update();
		m_enemies.Refresh();
		m_bgEffect.GetEffect().setSpeed(GetTimeScale());
		m_fgEffect.GetEffect().setSpeed(GetTimeScale());

		// 視界マスク更新
		m_caveVision.UpdateMask(m_player.CurrentPos().viewPos + Point(CellPx_24, CellPx_24) / 2);

		// ヒットストッピング管理
		SetTimeScale(m_hitStoppingRequested > 0 ? getToml<double>(U"hitstopping_timescale") : 1.0);
	}

private:
	static MapGrid generateMap()
	{
		// GenerateFreshMaze(MazeGenProps{
		// 	.size = {81, 81},
		// });
		return GenerateFreshDungeon(DungGenProps{
			.size = {81, 81},
			.areaDivision = 8,
		});
	}

	void ensureInitializedTransition()
	{
		if (m_floorTransition.IsInitialized()) return;
		m_floorTransition = m_ui.Birth(UiFloorTransition());
		m_floorTransition.Init();
	}

	void installEnemies(ActorView self)
	{
		for (int i = 0; i < 10; ++i)
		{
			auto enemy = m_enemies.Birth(self.AsParent(), EnSlimeCat());
			enemy.Init();
		}

		for (int i = 0; i < 5; ++i)
		{
			auto enemy = m_enemies.Birth(self.AsParent(), EnKnight());
			enemy.Init();
		}
	}
};

namespace Play
{
	std::unique_ptr<PlayScene> s_instance = nullptr;

	PlayScene::PlayScene() : p_impl(std::make_shared<Impl>())
	{
		s_instance = std::make_unique<PlayScene>(*this);
	}

	PlayScene::~PlayScene()
	{
		if (s_instance != nullptr && s_instance->p_impl == this->p_impl && p_impl.use_count() == 2)
			s_instance.reset(nullptr);
	}

	void PlayScene::Init(const PlaySingletonData& data)
	{
		p_impl->Init(*this, data);
	}

	void PlayScene::Update()
	{
		p_impl->UpdateScene(*this);
		p_impl->m_caveVision.UpdateScreen();
		p_impl->m_ui.Update();
	}

	void PlayScene::Kill()
	{
		ActorBase::Kill();
		p_impl->m_ui.Kill();
	}

	ActorWeak PlayScene::StartTransition(int floorIndex)
	{
		return p_impl->StartTransition(floorIndex);
	}

	ActorWeak PlayScene::EndTransition()
	{
		return p_impl->m_floorTransition.PerformClose();
	}

	PlayScene& PlayScene::Instance()
	{
		assert(s_instance != nullptr);
		return *s_instance;
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
			.playerPersonal = p_impl->m_player.PersonalData(),
			.timeLimiter = p_impl->m_uiTimeLimiter.GetData()
		};
	}
}
