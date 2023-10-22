#include "stdafx.h"
#include "PlayScene.h"

#include "Player.h"
#include "Map/BgMapDrawer.h"
#include "Map/DungeonGenerator.h"
#include "Map/MapGrid.h"
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
		Player m_player;
		Camera2D m_debugCamera;
		CameraKind::Value m_camera = CameraKind::Player;

		void UpdateScene(ActorBase& self)
		{
#ifdef  _DEBUG
			// デバッグ用にカメラ変更
			if (KeyNum0.down())
				m_camera =
					static_cast<CameraKind::Value>((m_camera + 1) % CameraKind::Max);
#endif
			if (m_camera == CameraKind::Debug) m_debugCamera.update();

			const auto t = m_camera == CameraKind::Player
				               ? Transformer2D(m_player.CameraTransform())
				               : m_debugCamera.createTransformer();

			const ScopedRenderStates2D sampler{SamplerState::BorderNearest};

			// 背景描画
			DrawBgMap(m_map);

			// キャラクターなどの通常更新
			self.ActorBase::Update();
		}
	};

	PlayScene* s_instance = nullptr;

	PlayScene::PlayScene() :
		p_impl(std::make_shared<Impl>())
	{
		s_instance = this;

		p_impl->m_map = GenerateFreshDungeon(DungGenProps{
			.size = {80, 80},
			.areaDivision = 8,
		});

		p_impl->m_player = AsParent().Birth(Player());

		p_impl->m_player.Init();
	}

	PlayScene::~PlayScene()
	{
		if (s_instance->p_impl == this->p_impl && p_impl.use_count() == 1) s_instance = nullptr;
	}

	void PlayScene::Update()
	{
		// if (MouseL.down())
		// {
		// 	p_impl->m_map = GenerateFreshDungeon(DungGenProps{
		// 		.size = {80, 80},
		// 		.areaDivision = 8,
		// 	});
		// }
		p_impl->UpdateScene(*this);
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
}
