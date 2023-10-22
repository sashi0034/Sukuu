#include "stdafx.h"
#include "Player.h"

#include "PlayScene.h"
#include "Character\CharacterUtil.h"
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
	CharacterPos m_actualPos{};
	Vec2 m_viewPos{};
	double m_moveSpeed = 1.0;
	double m_cameraScale = 4;
	bool m_dashing{};

	void Update()
	{
		(void)TextureAsset(AssetImages::phine_32x32)(playerRect)
			.draw(m_viewPos.movedBy(GetCharacterCellPadding(playerRect.size)));
	}

	void ProcessAsync(YieldExtended& yield, ActorBase& self)
	{
		while (true)
		{
			processLoop(yield, self);
		}
	}

private:
	double moveDuration() const { return 0.5 / (m_moveSpeed * (m_dashing ? 2.0 : 1.0)); }

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
			if (moveDir != Dir4::Invalid &&
				CanMoveTo(PlayScene::Instance().GetMap(), m_actualPos, moveDir))
				break;

			yield();
		}
		m_dashing = KeyShift.pressed();

		// 移動
		const auto nextPos = Vec2(m_actualPos + moveDir.ToXY() * CellPx_24);
		Util::AnimateEasing<EaseInLinear, EaseOption::Default>(
			self, &m_actualPos, CharacterPos(nextPos),
			moveDuration());
		yield.WaitForDead(
			Util::AnimateEasing<EaseInLinear, EaseOption::None>(
				self, &m_viewPos, nextPos,
				moveDuration())
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
		p_impl->m_actualPos = PlayScene::Instance().GetMap().Rooms().RandomRoomPoint() * CellPx_24;
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
		       .translated(-p_impl->m_viewPos - playerRect.size / 2)
		       .scaled(p_impl->m_cameraScale, Scene::Center());
	}
}
