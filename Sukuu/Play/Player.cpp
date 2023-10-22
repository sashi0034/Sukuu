#include "stdafx.h"
#include "Player.h"

#include "PlayScene.h"

namespace Play
{
	struct Player::Impl
	{
		Vec2 m_pos;

		void Update()
		{
			(void)TextureAsset(AssetImages::phine_32x32)(0, 0, 32, 32)
				.draw(m_pos.movedBy(-4, -8));
		}
	};

	Player::Player() : p_impl(std::make_shared<Impl>())
	{
	}

	void Player::Init()
	{
		p_impl->m_pos = PlayScene::Instance().GetMap().Rooms().RandomRoomPoint() * CellDots_24;
	}

	void Player::Update()
	{
		p_impl->Update();
	}
}
