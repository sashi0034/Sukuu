#include "stdafx.h"
#include "Player.h"

#include "PlayScene.h"
#include "Chara\CharaUtil.h"
#include "Player_detail/PlayerAnimation.h"
#include "Player_detail/PlayerDistField.h"
#include "Util/CoroUtil.h"
#include "Util/Dir4.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	constexpr Rect playerRect{0, 0, 32, 32};

	enum class PlayerAct
	{
		Idle,
		Walk,
		Running,
	};
}

struct Play::Player::Impl
{
	CharaPosition m_pos;
	double m_moveSpeed = 1.0;
	double m_cameraScale = 4;
	PlayerAct m_act = PlayerAct::Idle;
	AnimTimer m_animTimer{};
	Dir4Type m_direction{Dir4::Down};
	PlayerDistFieldInternal m_distField{};
	bool m_completedGoal{};

	void Update()
	{
		m_animTimer.Tick(Scene::DeltaTime() * (m_act == PlayerAct::Running ? 2 : 1));

		(void)GetPlayerTexture(playerRect, m_direction, m_animTimer, isWalking())
			.draw(m_pos.viewPos.movedBy(GetCharacterCellPadding(playerRect.size)));
	}

	void FlowchartAsync(YieldExtended& yield, ActorBase& self)
	{
		while (true)
		{
			flowchartLoop(yield, self);
		}
	}

private:
	bool isWalking() const
	{
		return m_act == PlayerAct::Walk || m_act == PlayerAct::Running;
	}

	double moveDuration() const
	{
		return GetTomlParameter<double>(U"play.player.move_duration")
			/ (m_moveSpeed * (m_act == PlayerAct::Running ? 2.0 : 1.0));
	}

	void flowchartLoop(YieldExtended& yield, ActorBase& self)
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
				CanMoveTo(PlayScene::Instance().GetMap(), m_pos.actualPos, moveDir))
				break;

			if (m_act != PlayerAct::Idle)
			{
				m_act = PlayerAct::Idle;
				m_animTimer.Reset();
			}

			yield();
		}

		// 以下、入力方向が決定済み

		if (m_act == PlayerAct::Idle)
		{
			m_animTimer.Reset();
		}
		m_act = KeyShift.pressed() ? PlayerAct::Running : PlayerAct::Walk;
		m_direction = moveDir;

		// 移動
		const auto nextPos = Vec2(m_pos.actualPos + moveDir.ToXY() * CellPx_24);
		ProcessMoveCharaPos(yield, self, m_pos, nextPos, moveDuration());
		m_distField.Refresh(PlayScene::Instance().GetMap(), nextPos);

		// ギミックチェック
		const auto newPoint = CharaVec2(nextPos).MapPoint();
		checkGimmickAt(yield, self, newPoint);
	}

	void checkGimmickAt(YieldExtended& yield, ActorBase& self, const Point newPoint)
	{
		const auto storedAct = m_act;
		m_act = PlayerAct::Idle;

		switch (auto checkingGimmick = PlayScene::Instance().GetGimmick()[newPoint])
		{
		case GimmickKind::Stairs:
			// ゴール到達
			yield.WaitForDead(
				AnimateEasing<EaseInBack>(self, &m_cameraScale, 8.0, 0.5));
			yield.WaitForDead(
				AnimateEasing<EaseOutCirc>(self, &m_cameraScale, 10.0, 0.5));
			m_completedGoal = true;
			break;
		default: ;
			m_act = storedAct;
			break;
		}
	}
};

namespace Play
{
	Player::Player() : p_impl(std::make_shared<Impl>())
	{
	}

	void Player::Init()
	{
		p_impl->m_pos.SetPos(GetInitialPos(PlayScene::Instance().GetMap()));

		p_impl->m_distField.Resize(PlayScene::Instance().GetMap().Data().size());

		StartCoro(*this, [*this](YieldExtended yield) mutable
		{
			p_impl->FlowchartAsync(yield, *this);
		});
	}

	void Player::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	double Player::OrderPriority() const
	{
		return CharaOrderPriority(p_impl->m_pos);
	}

	Mat3x2 Player::CameraTransform() const
	{
		return Mat3x2::Translate({Scene::Center()})
		       .translated(-p_impl->m_pos.viewPos - playerRect.size / 2)
		       .scaled(p_impl->m_cameraScale, Scene::Center());
	}

	const PlayerDistField& Player::DistField() const
	{
		return p_impl->m_distField.Field();
	}

	bool Player::IsCompletedGoal() const
	{
		return p_impl->m_completedGoal;
	}
}
