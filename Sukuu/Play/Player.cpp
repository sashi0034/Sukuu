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
		Dead,
	};
}

struct Play::Player::Impl
{
	CoroActor m_flowchart{};

	CharaPosition m_pos;
	Vec2 m_animOffset{};
	double m_moveSpeed = 1.0;
	double m_cameraScale = 4;
	PlayerAct m_act = PlayerAct::Idle;
	AnimTimer m_animTimer{};
	Dir4Type m_direction{Dir4::Down};
	PlayerDistFieldInternal m_distField{};
	bool m_completedGoal{};
	bool m_isImmortal{};
	std::function<void()> m_scoopDrawing = {};

	void Update()
	{
		m_animTimer.Tick(GetDeltaTime() * (m_act == PlayerAct::Running ? 2 : 1));

		if (m_scoopDrawing) m_scoopDrawing();;
		m_scoopDrawing = {};

		const auto drawingPos = m_pos.viewPos.movedBy(GetCharacterCellPadding(playerRect.size) + m_animOffset);
		(void)getPlayerTexture()
			.draw(drawingPos);
	}

	void StartFlowchart(ActorBase& self)
	{
		m_flowchart = StartCoro(self, [this, self](YieldExtended yield) mutable
		{
			while (true)
			{
				flowchartLoop(yield, self);
			}
		});
	}

	void EnemyCollide(ActorBase& self, const RectF& enemy)
	{
		if (m_isImmortal) return;;
		if (m_act == PlayerAct::Dead) return;

		const auto player = RectF{m_pos.actualPos, playerRect.size}.stretched(
			GetTomlParameter<int>(U"play.player.collider_padding"));

		if (enemy.intersects(player) == false) return;
		// 以下、当たった状態

		m_act = PlayerAct::Dead;
		m_flowchart.Kill();
		m_distField.Clear();

		// やられた演出
		StartCoro(self, [this, self](YieldExtended yield) mutable
		{
			SetTimeScale(0.01);
			yield.WaitForTime(0.5, Scene::DeltaTime);
			SetTimeScale(1);

			AnimateEasing<BoomerangParabola>(
				self,
				&m_animOffset,
				GetTomlParameter<Vec2>(U"play.en_slime_cat.dead_animation_offset"),
				GetTomlParameter<double>(U"play.en_slime_cat.dead_animation_duration"));
			yield.WaitForTime(GetTomlParameter<double>(U"play.en_slime_cat.dead_pause_duration"));
			StartFlowchart(self);
		});
	}

	Mat3x2 CameraTransform() const
	{
		return Mat3x2::Translate({Scene::Center()})
		       .translated(-m_pos.viewPos - playerRect.size / 2)
		       .scaled(m_cameraScale, Scene::Center());
	}

private:
	TextureRegion getPlayerTexture() const
	{
		if (m_act == PlayerAct::Dead) return GetDeadPlayerTexture(playerRect);
		return GetUsualPlayerTexture(playerRect, m_direction, m_animTimer, isWalking());
	}

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
		if (m_act == PlayerAct::Dead)
		{
			m_act = PlayerAct::Idle;
			m_distField.Refresh(PlayScene::Instance().GetMap(), m_pos.actualPos);
		}

		// キー入力待ち
		auto moveDir = Dir4::Invalid;
		while (true)
		{
			moveDir = checkMoveInput();
			if (moveDir != Dir4::Invalid &&
				CanMoveTo(PlayScene::Instance().GetMap(), m_pos.actualPos, moveDir))
				break;

			if (m_act != PlayerAct::Idle)
			{
				m_act = PlayerAct::Idle;
				m_animTimer.Reset();
			}

			// プレイヤーを掬おうとしてるかチェック
			checkScoopFromMouse(yield, self);

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

	static Dir4Type checkMoveInput()
	{
		if (KeyW.pressed()) return Dir4::Up;
		if (KeyA.pressed()) return Dir4::Left;
		if (KeyS.pressed()) return Dir4::Down;
		if (KeyD.pressed()) return Dir4::Right;
		return Dir4::Invalid;
	}

	void checkScoopFromMouse(YieldExtended& yield, ActorBase& self)
	{
		if (MouseL.pressed()) return;
		if (RectF(m_pos.actualPos, {CellPx_24, CellPx_24}).intersects(Cursor::PosF()) == false)
			return;
		// 以下、マウスカーソルが当たった状態

		// マウスクリックまで待機
		while (true)
		{
			if (checkMoveInput() != Dir4::Invalid) return;
			if (RectF(m_pos.actualPos, {CellPx_24, CellPx_24}).intersects(Cursor::PosF()) == false)
				return;

			m_scoopDrawing = [this]()
			{
				RectF(m_pos.actualPos, {CellPx_24, CellPx_24})
					.draw(GetTomlParameter<ColorF>(U"play.en_slime_cat.scoop_rect_color_1"));
			};

			yield();
			if (MouseL.down()) break;
		}

		// ドラッグ解除まで待機
		while (true)
		{
			m_scoopDrawing = [this]()
			{
				for (int i = 0; i < 4; ++i)
				{
					auto r = RectF(m_pos.actualPos.movedBy(Dir4Type(i).ToXY() * CellPx_24), {CellPx_24, CellPx_24});
					r.draw(GetTomlParameter<ColorF>(U"play.en_slime_cat.scoop_rect_color_2"));
				}
			};

			yield();
			if (MouseL.pressed() == false) break;

			// 目標が決まったかチェック
			for (int i = 0; i < 4; ++i)
			{
				const auto checkingPos = m_pos.actualPos.movedBy(Dir4Type(i).ToXY() * CellPx_24);
				auto r = RectF(checkingPos, {CellPx_24, CellPx_24});
				if (r.intersects(Cursor::PosF()) == false) continue;

				// 移動させる
				m_distField.Clear();
				m_isImmortal = true;
				const double animDuration = GetTomlParameter<double>(U"play.en_slime_cat.scoop_move_duration");
				AnimateEasing<BoomerangParabola>(self, &m_animOffset, Vec2{0, -32}, animDuration);
				ProcessMoveCharaPos(
					yield, self, m_pos, checkingPos,
					animDuration);
				m_isImmortal = false;
				goto dropped;
			}
		}

	dropped:;
	}

	void checkGimmickAt(YieldExtended& yield, ActorBase& self, const Point newPoint)
	{
		const auto storedAct = m_act;
		m_act = PlayerAct::Idle;

		switch (auto checkingGimmick = PlayScene::Instance().GetGimmick()[newPoint])
		{
		case GimmickKind::Stairs: {
			m_isImmortal = true;
			// ゴール到達
			yield.WaitForDead(
				AnimateEasing<EaseInBack>(self, &m_cameraScale, 8.0, 0.5));
			yield.WaitForDead(
				AnimateEasing<EaseOutCirc>(self, &m_cameraScale, 10.0, 0.5));
			m_completedGoal = true;
			break;
		}
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

		p_impl->StartFlowchart(*this);
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

	void Player::SendEnemyCollide(const RectF& rect)
	{
		p_impl->EnemyCollide(*this, rect);
	}

	Mat3x2 Player::CameraTransform() const
	{
		return p_impl->CameraTransform();
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
