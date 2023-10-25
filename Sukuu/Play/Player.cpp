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
	ActorBase m_focusAnimation{};

	PlayerPersonalData m_personal{};
	CharaPosition m_pos;
	Vec2 m_animOffset{};
	double m_moveSpeed = 1.0;
	double m_cameraScale = 4.0;
	double m_focusCameraRate = 1.0;
	Vec2 m_cameraOffset{};
	Vec2 m_cameraOffsetDestination{};
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

		if (m_scoopDrawing)
		{
			m_scoopDrawing();
		}
		else
		{
			// ドラッグ中じゃないなら、カメラのオフセットを動かす
			m_cameraOffset += (m_cameraOffsetDestination - m_cameraOffset)
				* Scene::DeltaTime()
				* GetTomlParameter<double>(U"play.player.camera_offset_speed");
		}

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

	// エネミーとの衝突判定
	void EnemyCollide(ActorBase& self, const RectF& enemy)
	{
		if (m_isImmortal) return;;
		if (m_act == PlayerAct::Dead) return;

		const auto player = RectF{m_pos.actualPos, playerRect.size}.stretched(
			GetTomlParameter<int>(U"play.player.collider_padding"));

		if (enemy.intersects(player) == false) return;
		// 以下、当たった状態

		m_act = PlayerAct::Dead;
		breakFlowchart();
		focusCameraFor<EaseOutBack>(self, GetTomlParameter<double>(U"play.player.focus_scale_large"));

		// やられた演出
		StartCoro(self, [this, self](YieldExtended yield) mutable
		{
			AnimatePlayerDie(yield, self, m_animOffset);

			focusCameraFor<EaseInOutBack>(self, 1.0);
			StartFlowchart(self);
		});
	}

	bool UseItem(ActorBase& self, ConsumableItem item)
	{
		if (m_act != PlayerAct::Idle) return false;

		switch (item)
		{
		case ConsumableItem::None:
			break;
		case ConsumableItem::Wing:
			return gotoStairsByWing(self);
		case ConsumableItem::Helmet:
			break;
		case ConsumableItem::Pin:
			break;
		case ConsumableItem::Max:
			break;
		default: ;
		}
		return false;
	}

	Mat3x2 CameraTransform() const
	{
		return Mat3x2::Translate({Scene::Center()})
		       .translated(-m_pos.viewPos + m_cameraOffset - Vec2{CellPx_24, CellPx_24} / 2)
		       .scaled(m_cameraScale * m_focusCameraRate, Scene::Center());
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

	void breakFlowchart()
	{
		m_cameraOffsetDestination = {0, 0};
		m_flowchart.Kill();
		m_distField.Clear();
		m_scoopDrawing = {};
	}

	void flowchartLoop(YieldExtended& yield, ActorBase& self)
	{
		if (m_act == PlayerAct::Dead)
		{
			m_act = PlayerAct::Idle;
		}
		m_distField.Refresh(PlayScene::Instance().GetMap(), m_pos.actualPos);

		// キー入力待ち
		auto moveDir = Dir4::Invalid;
		while (true)
		{
			// プレイヤーを掬おうとしてるかチェック
			checkScoopFromMouse(yield, self);

			moveDir = checkMoveInput();
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
		m_cameraOffsetDestination = -moveDir.ToXY() * GetTomlParameter<double>(U"play.player.camera_offset_amount");

		// 移動
		const auto nextPos = Vec2(m_pos.actualPos + moveDir.ToXY() * CellPx_24);
		ProcessMoveCharaPos(yield, self, m_pos, nextPos, moveDuration());
		// m_distField.Refresh(PlayScene::Instance().GetMap(), nextPos);

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

	template <double easing(double) = EaseInOutSine>
	void focusCameraFor(ActorBase& self, double scale)
	{
		m_focusAnimation.Kill();
		AnimateEasing<easing>(self, &m_focusCameraRate, scale, 0.5);
	}

	void checkScoopFromMouse(YieldExtended& yield, ActorBase& self)
	{
		if (RectF(m_pos.actualPos, {CellPx_24, CellPx_24}).intersects(Cursor::PosF()) == false)
			return;
		// 以下、マウスカーソルが当たった状態

		// マウスクリックまで待機
		focusCameraFor(self, GetTomlParameter<double>(U"play.player.focus_scale_large"));
		m_scoopDrawing = [this, self]() mutable
		{
			if (RectF(m_pos.actualPos, {CellPx_24, CellPx_24}).intersects(Cursor::PosF()) == false)
			{
				// 解除
				m_scoopDrawing = {};
				focusCameraFor(self, 1.0);
				return;
			}
			RectF(m_pos.actualPos.MapPoint() * CellPx_24, {CellPx_24, CellPx_24})
				.draw(GetTomlParameter<ColorF>(U"play.player.scoop_rect_color_1"));
		};
		while (true)
		{
			if (checkMoveInput() != Dir4::Invalid && MouseL.pressed() == false) return;
			if (RectF(m_pos.actualPos, {CellPx_24, CellPx_24}).intersects(Cursor::PosF()) == false)
				return;

			yield();
			if (MouseL.pressed()) break;
		}

		// ドラッグ解除まで待機
		m_scoopDrawing = [this]()
		{
			for (int i = 0; i < 4; ++i)
			{
				auto r = RectF(m_pos.actualPos.movedBy(Dir4Type(i).ToXY() * CellPx_24), {CellPx_24, CellPx_24});
				r.draw(GetTomlParameter<ColorF>(U"play.player.scoop_rect_color_2"));
			}
		};
		while (true)
		{
			yield();
			if (MouseL.pressed() == false)
			{
				m_scoopDrawing = {};
				break;
			}

			// 目標が決まったかチェック
			for (int i = 0; i < 4; ++i)
			{
				const auto checkingPos = m_pos.actualPos.movedBy(Dir4Type(i).ToXY() * CellPx_24);
				auto r = RectF(checkingPos, {CellPx_24, CellPx_24});
				if (r.intersects(Cursor::PosF()) == false) continue;

				// 移動させる
				// m_distField.Clear();
				m_scoopDrawing = {};
				m_isImmortal = true;
				const double animDuration = GetTomlParameter<double>(U"play.player.scoop_move_duration");
				AnimateEasing<BoomerangParabola>(self, &m_animOffset, Vec2{0, -32}, animDuration);
				ProcessMoveCharaPos(
					yield, self, m_pos, checkingPos,
					animDuration);
				m_distField.Refresh(PlayScene::Instance().GetMap(), m_pos.actualPos);
				m_isImmortal = false;
				goto dropped;
			}
		}

	dropped:;
		focusCameraFor(self, 1.0);
		yield.WaitForTrue([]()
		{
			return MouseL.pressed() == false;
		});
	}

	void checkGimmickAt(YieldExtended& yield, ActorBase& self, const Point newPoint)
	{
		const auto storedAct = m_act;
		m_act = PlayerAct::Idle;

		auto&& gimmickGrid = PlayScene::Instance().GetGimmick();
		switch (gimmickGrid[newPoint])
		{
		case GimmickKind::Stairs: {
			m_cameraOffsetDestination = {0, 0};
			m_isImmortal = true;
			// ゴール到達
			yield.WaitForDead(
				AnimateEasing<EaseInBack>(self, &m_cameraScale, 8.0, 0.5));
			yield.WaitForDead(
				AnimateEasing<EaseOutCirc>(self, &m_cameraScale, 10.0, 0.5));
			m_completedGoal = true;
			break;
		}
		case GimmickKind::Item_Wing: [[fallthrough]];
		case GimmickKind::Item_Helmet: [[fallthrough]];
		case GimmickKind::Item_Pin:
			obtainItemAt(newPoint, gimmickGrid);
			break;
		default: ;
			m_act = storedAct;
			break;
		}
	}

	void obtainItemAt(const Point point, GimmickGrid& gimmickGrid)
	{
		for (int i = 0; i < m_personal.items.size(); ++i)
		{
			if (m_personal.items[i] != ConsumableItem::None) continue;

			// アイテム入手
			m_personal.items[i] = GimmickToItem(gimmickGrid[point]);
			assert(m_personal.items[i] != ConsumableItem::None);
			gimmickGrid[point] = GimmickKind::None;
			break;
		}
	}

	bool gotoStairsByWing(ActorBase& self)
	{
		breakFlowchart();
		StartCoro(self, [this, self](YieldExtended yield) mutable
		{
			AnimatePlayerUsingWing(yield, self, m_animOffset, m_pos);
			StartFlowchart(self);
		});
		return true;
	}
};

namespace Play
{
	Player::Player() : p_impl(std::make_shared<Impl>())
	{
	}

	void Player::Init(const PlayerPersonalData& data)
	{
		p_impl->m_personal = data;

		p_impl->m_pos.SetPos(GetInitialPos(PlayScene::Instance().GetMap()));

		p_impl->m_distField.Resize(PlayScene::Instance().GetMap().Data().size());

		p_impl->StartFlowchart(*this);

#ifdef _DEBUG
		// p_impl->m_personal.items[0] = ConsumableItem::Wing;
		// p_impl->m_personal.items[1] = ConsumableItem::Helmet;
#endif
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

	bool Player::RequestUseItem(int itemIndex)
	{
		const bool used = p_impl->UseItem(*this, p_impl->m_personal.items[itemIndex]);
		if (used)
		{
			p_impl->m_personal.items[itemIndex] = ConsumableItem::None;
		}
		return used;
	}

	const PlayerPersonalData& Player::PersonalData() const
	{
		return p_impl->m_personal;
	}

	Mat3x2 Player::CameraTransform() const
	{
		return p_impl->CameraTransform();
	}

	const CharaPosition& Player::CurrentPos() const
	{
		return p_impl->m_pos;
	}

	Point Player::CurrentPoint() const
	{
		return p_impl->m_pos.actualPos.MapPoint();
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
