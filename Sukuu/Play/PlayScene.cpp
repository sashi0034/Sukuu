#include "stdafx.h"
#include "PlayScene.h"

#include "Player.h"
#include "Map/AutoTiler.h"
#include "Map/BgMapDrawer.h"
#include "Map/DungeonGenerator.h"
#include "Map/MapGrid.h"
#include "Util/ActorContainer.h"

namespace Play
{
	class PlayScene::Impl
	{
	public:
		MapGrid m_map;
		Player m_player;
		Camera2D m_camera;

		void UpdateScene(ActorBase& self)
		{
			m_camera.update();
			const auto t = m_camera.createTransformer();
			const ScopedRenderStates2D sampler{SamplerState::BorderNearest};

			DrawBgMap(m_map);
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
		s_instance = nullptr;
	}

	void PlayScene::Update()
	{
		if (MouseL.down())
		{
			p_impl->m_map = GenerateFreshDungeon(DungGenProps{
				.size = {80, 80},
				.areaDivision = 8,
			});
		}
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
