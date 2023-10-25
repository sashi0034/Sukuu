#include "stdafx.h"
#include "PlayScene.h"

#include "Player.h"
#include "Enemy/EnSlimeCat.h"
#include "Gimmick/GimmickInstaller.h"
#include "Map/BgMapDrawer.h"
#include "Map/DungeonGenerator.h"
#include "Map/MapGrid.h"
#include "Map/MazeGenerator.h"
#include "Other/CaveVision.h"
#include "UI/UiItemContainer.h"
#include "Util/ActorContainer.h"

namespace Play
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

	class PlayScene::Impl
	{
	public:
		MapGrid m_map;
		GimmickGrid m_gimmick;
		BgMapDrawer m_bgMapDrawer{};
		Player m_player;
		Camera2D m_debugCamera;
		CameraKind::Value m_camera = CameraKind::Player;
		ActorContainer m_ui{};
		UiItemContainer m_uiItemContainer;
		CaveVision m_caveVision{};

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

			// 視界マスク更新
			m_caveVision.UpdateMask(m_player.CurrentPos().viewPos + Point(CellPx_24, CellPx_24) / 2);
		}
	};

	PlayScene* s_instance = nullptr;

	PlayScene::PlayScene() : PlayScene(PlaySingletonData{}) { return; }

	PlayScene::PlayScene(const PlaySingletonData& data) :
		p_impl(std::make_shared<Impl>())
	{
		s_instance = this;

		p_impl->m_map = GenerateFreshDungeon(DungGenProps{
			.size = {81, 81},
			.areaDivision = 8,
		});
		// p_impl->m_map = GenerateFreshMaze(MazeGenProps{
		// 	.size = {81, 81},
		// });

		p_impl->m_gimmick.resize(p_impl->m_map.Data().size(), GimmickKind::None);
		InstallGimmicks(p_impl->m_gimmick, p_impl->m_map);

		// 生成
		p_impl->m_player = AsParent().Birth(Player());

		// TODO: EnemyManager
		for (int i = 0; i < 10; ++i)
		{
			auto enemy = AsParent().Birth(EnSlimeCat());
			enemy.Init();
		}

		p_impl->m_uiItemContainer = p_impl->m_ui.Birth(UiItemContainer());

		// 初期化
		p_impl->m_player.Init(data.playerPersonal);
	}

	PlayScene::~PlayScene()
	{
		if (s_instance->p_impl == this->p_impl && p_impl.use_count() == 1) s_instance = nullptr;
	}

	void PlayScene::Update()
	{
		p_impl->UpdateScene(*this);
		p_impl->m_caveVision.UpdateScreen();
		p_impl->m_ui.Update();
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

	PlaySingletonData PlayScene::CopyData() const
	{
		return PlaySingletonData{
			.playerPersonal = p_impl->m_player.PersonalData()
		};
	}
}
