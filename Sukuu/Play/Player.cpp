#include "stdafx.h"
#include "Player.h"

#include "PlayScene.h"
#include "Util/ActorContainer.h"
#include "Util/CoroUtil.h"
#include "Util/Dir4.h"
#include "Util/EasingAnimation.h"

struct Play::Player::Impl
{
	Vec2 m_pos;
	double m_moveSpeed = 1.0;

	void Update()
	{
		(void)TextureAsset(AssetImages::phine_32x32)(0, 0, 32, 32)
			.draw(m_pos.movedBy(-4, -8));
	}

	void ProcessAsync(YieldExtended& yield, ActorBase& self)
	{
		while (true)
		{
			processLoop(yield, self);
		}
	}

private:
	double moveDuration() const { return 0.5 / m_moveSpeed; }

	void processLoop(YieldExtended& yield, ActorBase& self)
	{
		// キー入力待ち
		auto moveDir = Dir4::Invalid;
		while (true)
		{
			yield();

			if (KeyW.pressed()) moveDir = Dir4::Up;
			if (KeyA.pressed()) moveDir = Dir4::Left;
			if (KeyS.pressed()) moveDir = Dir4::Down;
			if (KeyD.pressed()) moveDir = Dir4::Right;
			if (moveDir != Dir4::Invalid) break;
		}

		// 移動
		yield.WaitForDead(
			Util::AnimateEasing<EaseInLinear>(self, &m_pos, m_pos + moveDir.ToXY() * CellPx_24, moveDuration())
		);
	}
};

namespace Play
{
	Player::Player() : p_impl(std::make_shared<Impl>())
	{
	}

	void Player::Init()
	{
		p_impl->m_pos = PlayScene::Instance().GetMap().Rooms().RandomRoomPoint() * CellPx_24;
		StartCoro(*this, [&](YieldExtended yield)
		{
			p_impl->ProcessAsync(yield, *this);
		});
	}

	void Player::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}
}
