#include "stdafx.h"
#include "Player.h"

#include "PlayScene.h"
#include "Util/ActorContainer.h"
#include "Util/CoroUtil.h"
#include "Util/Dir4.h"
#include "Util/EasingAnimation.h"

namespace
{
	constexpr Rect playerRect{0, 0, 32, 32};
}

struct Play::Player::Impl
{
	Vec2 m_pos;
	double m_moveSpeed = 1.0;
	double m_cameraScale = 4;

	void Update()
	{
		(void)TextureAsset(AssetImages::phine_32x32)(playerRect)
			.draw(m_pos.movedBy(GetCharacter24Padding(playerRect.size)));
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
			if (KeyW.pressed()) moveDir = Dir4::Up;
			if (KeyA.pressed()) moveDir = Dir4::Left;
			if (KeyS.pressed()) moveDir = Dir4::Down;
			if (KeyD.pressed()) moveDir = Dir4::Right;
			if (moveDir != Dir4::Invalid) break;

			yield();
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

	Mat3x2 Player::CameraTransform() const
	{
		return Mat3x2::Translate({Scene::Center()})
		       .translated(-p_impl->m_pos - playerRect.size / 2)
		       .scaled(p_impl->m_cameraScale, Scene::Center());
	}
}
