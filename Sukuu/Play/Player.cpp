#include "stdafx.h"
#include "Player.h"

#include "PlayScene.h"
#include "Chara/CharaUtil.h"
#include "Item/ItemGrave.h"
#include "Item/ItemMagnet.h"
#include "Item/ItemMine.h"
#include "Item/ItemPin.h"
#include "Item/ItemSun.h"
#include "Other/PlayPenaltyBonus.h"
#include "Player_detail/PlayerAnimation.h"
#include "Player_detail/PlayerDistField.h"
#include "Player_detail/PlayerInternal.h"
#include "Util/CoroUtil.h"
#include "Util/Dir4.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.player." + key);
	}

	constexpr double defaultCameraScale = 4.0;
}

struct Play::Player::Impl
{
	CoroWeak m_flowchart{};

	PlayerPersonalData m_personal{};
	CharaPosition m_pos;
	Vec2 m_animOffset{};
	double m_moveSpeed = 1.0;
	double m_cameraScale = defaultCameraScale;
	double m_focusCameraRate = 1.0;
	Vec2 m_cameraOffset{};
	Vec2 m_cameraOffsetDestination{};
	PlayerAct m_act = PlayerAct::Idle;
	AnimTimer m_animTimer{};
	Dir4Type m_direction{Dir4::Down};
	PlayerDistFieldInternal m_distField{};
	bool m_terminated{};
	PlayerImmortality m_immortal{};
	bool m_guardHelmet{};
	std::function<void()> m_scoopDrawing = {};
	int m_scoopContinuous{};
	PlayerVisionState m_vision{};
	double m_faintStealthTime{};
	bool m_isGameOver{};

	void Update()
	{
#if _DEBUG
		if (KeyNum1.down()) m_vision.mistRemoval = not m_vision.mistRemoval;
#endif
		updateVision(m_vision, m_act);

		m_immortal.immortalTime = std::max(m_immortal.immortalTime - GetDeltaTime(), 0.0);

		m_animTimer.Tick(GetDeltaTime() * (m_act == PlayerAct::Running ? 2 : 1));

		m_faintStealthTime = std::max(m_faintStealthTime - GetDeltaTime(), 0.0);

		if (m_scoopDrawing)
		{
			// すくうの描画
			m_scoopDrawing();
		}
		else
		{
			// ドラッグ中じゃないなら、カメラのオフセットを動かす
			m_cameraOffset += (m_cameraOffsetDestination - m_cameraOffset)
				* Scene::DeltaTime()
				* getToml<double>(U"camera_offset_speed");
		}

		const ScopedRenderStates2D rs = getRenderState();

		const auto drawingPos = getDrawPos();
		(void)getPlayerTexture()
			.draw(drawingPos, ColorF{1.0, getTextureAlpha()});

		// ヘルメット
		if (m_guardHelmet)
		{
			TextureAsset(AssetImages::helmet_16x16)(0, 0, 16, 16).drawAt(drawingPos + getHelmetOffset());
		}
	}

	void CheckGameOver(ActorView self)
	{
		if (m_isGameOver) return;
		if (PlayScene::Instance().GetTimeLimiter().GetData().remainingTime > 0) return;

		// 以下、ゲームオーバー処理開始
		AudioAsset(AssetSes::fall_down).playOneShot();
		m_isGameOver = true;
		m_flowchart.Kill();
		m_distField.Clear();
		m_scoopDrawing = {};
		m_immortal.immortalStock++;
		AnimateEasing<EaseOutCirc>(self, &m_cameraScale, 8.0, 0.5);
		StartCoro(self, [this](YieldExtended yield)
		{
			yield.WaitForTime(1.0);
			yield.WaitForExpire(PlayScene::Instance().PerformGameOver());
			m_terminated = true;
		});
	}

	void StartFlowchart(ActorView self)
	{
		m_flowchart = StartCoro(self, [this, self](YieldExtended yield) mutable
		{
			while (true)
			{
				flowchartLoop(yield, self);
			}
		});
	}

	void PerformInitialCamera(ActorView self)
	{
		m_cameraScale = 10.0;
		AnimateEasing<EaseOutBack>(self, &m_cameraScale, defaultCameraScale, 0.5);
	}

	void PerformTutorialOpening(ActorView self)
	{
		StartCoro(self, [this, self](YieldExtended yield) mutable
		{
			// チュートリアルの最初
			m_flowchart.Kill();
			m_act = PlayerAct::Dead;
			m_cameraScale = 10.0;
			m_cameraOffset = {0, 1200};
			constexpr double duration = 8.0;
			AnimateEasing<EaseOutCubic>(self, &m_cameraOffset, {0, 0}, duration);
			yield.WaitForDead(
				AnimateEasing<EaseInOutBack>(self, &m_cameraScale, defaultCameraScale, duration));
			StartFlowchart(self);
		});
	}

	// エネミーとの衝突判定
	void EnemyCollide(ActorView self, const RectF& rect, EnemyKind enemy)
	{
		if (m_immortal.IsImmortal()) return;;
		if (m_act == PlayerAct::Dead) return;
		if (PlayScene::Instance().Tutorial() != nullptr) return; // チュートリアル中は無敵

		const auto player = RectF{m_pos.actualPos, PlayerCellRect.size}.stretched(
			getToml<int>(U"collider_padding"));

		if (rect.intersects(player) == false) return;
		// 以下、当たった状態

		if (m_guardHelmet)
		{
			// ヘルメットガード発動
			AudioAsset(AssetSes::break_helmet).playOneShot();
			m_guardHelmet = false;
			m_immortal.immortalTime = 1.0;
			EffectHelmetConsume(getDrawPos() + getHelmetOffset(), getPlayerTexture());
			return;
		}

		AudioAsset(AssetSes::damaged).playOneShot();
		m_act = PlayerAct::Dead;
		breakFlowchart();
		focusCameraFor<EaseOutBack>(self, getToml<double>(U"focus_scale_large"));

		// ペナルティとして時間減らす
		RelayTimeDamageAmount(m_pos, GetEnemyAttackDamage(enemy), true);

		// やられた演出
		StartCoro(self, [this, self](YieldExtended yield) mutable
		{
			AnimatePlayerDie(yield, self, m_animOffset, m_cameraOffset);

			focusCameraFor<EaseInOutBack>(self, 1.0);
			m_immortal.immortalTime = getToml<double>(U"recover_immortal");
			StartFlowchart(self);
		});
	}

	bool UseItem(ActorView self, ConsumableItem item)
	{
		switch (item)
		{
		case ConsumableItem::None:
			break;
		case ConsumableItem::Wing:
			if (m_act != PlayerAct::Idle) return false;
			return gotoStairsByWing(self);
		case ConsumableItem::Helmet: {
			if (m_guardHelmet) return false;
			m_guardHelmet = true;
			return true;
		}
		case ConsumableItem::Pin: {
			if (m_act != PlayerAct::Idle) return false;
			auto pin = PlayScene::Instance().AsParent().Birth(ItemPin());
			pin.Init(m_pos.actualPos, m_direction);
			return true;
		}
		case ConsumableItem::Mine: {
			if (canInstallGimmickNow() == false) return false;
			auto mine = PlayScene::Instance().AsParent().Birth(ItemMine());
			mine.Init(m_pos.actualPos);
			return true;
		}
		case ConsumableItem::LightBulb:
			return CheckUseItemLightBulb(self, m_vision);
		case ConsumableItem::Magnet: {
			if (canInstallGimmickNow() == false) return false;
			auto magnet = PlayScene::Instance().AsParent().Birth(ItemMagnet());
			magnet.Init(m_pos.actualPos);
			return true;
		}
		case ConsumableItem::Bookmark:
			return PlayScene::Instance().GetMiniMap().SpotStairsAndAllItems();
		case ConsumableItem::Explorer:
			return CheckUseItemExplorer(self);
		case ConsumableItem::Grave: {
			if (canInstallGimmickNow() == false) return false;
			auto grave = PlayScene::Instance().AsParent().Birth(ItemGrave());
			grave.Init(m_pos.actualPos);
			return true;
		}
		case ConsumableItem::Sun: {
			if (m_act != PlayerAct::Idle) return false;
			auto sun = PlayScene::Instance().AsParent().Birth(ItemSun());
			sun.Init(m_pos.actualPos, m_direction);
			return true;
		}
		case ConsumableItem::Tube: {
			if (m_act == PlayerAct::Dead) return false;
			RelayTimeHealAmount(m_pos, getToml<int>(U"tube_heal_amount"));
			return true;
		}
		case ConsumableItem::Solt: {
			if (m_faintStealthTime > 0) return false;
			m_faintStealthTime += getToml<double>(U"solt_faint");
			refreshDistField();
			return true;
		}
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
	Vec2 getDrawPos() const
	{
		return m_pos.viewPos.movedBy(GetCharacterCellPadding(PlayerCellRect.size) + m_animOffset);
	}

	ScopedRenderStates2D getRenderState() const
	{
		if (m_faintStealthTime > 0) return ScopedRenderStates2D(BlendState::Additive);
		return ScopedRenderStates2D{Graphics2D::GetBlendState()};
	}

	Vec2 getHelmetOffset() const
	{
		return PlayerCellRect.size / 2
			+ getToml<Point>(U"helmet_offset")
			+ Point{0, Math::Abs(3 - m_animTimer.SliceFrames(getToml<int>(U"helmet_animation"), 6))};
	}

	TextureRegion getPlayerTexture() const
	{
		if (m_act == PlayerAct::Dead || m_isGameOver) return GetDeadPlayerTexture();
		return GetUsualPlayerTexture(m_direction, m_animTimer, isWalking());
	}

	double getTextureAlpha() const
	{
		if (m_immortal.immortalTime > 0)
		{
			return (static_cast<int>(Scene::Time() * 1000) % 200 < 100)
				       ? 0.9
				       : 0.1;
		}
		return 1.0;
	}

	bool isWalking() const
	{
		return m_act == PlayerAct::Walk || m_act == PlayerAct::Running;
	}

	double moveDuration() const
	{
		return getToml<double>(U"move_duration")
			/ (m_moveSpeed * (m_act == PlayerAct::Running ? 2.0 : 1.0));
	}

	void breakFlowchart()
	{
		m_cameraOffsetDestination = {0, 0};
		m_flowchart.Kill();
		m_distField.Clear();
		m_scoopDrawing = {};
	}

	void flowchartLoop(YieldExtended& yield, ActorView self)
	{
		while (m_isGameOver)
		{
			yield();
		}
		if (m_act == PlayerAct::Dead)
		{
			m_act = PlayerAct::Idle;
		}
		refreshDistField();

		// キー入力待ち
		auto moveDir = Dir4::Invalid;
		while (true)
		{
			// プレイヤーを掬おうとしてるかチェック
			checkScoopFromMouse(yield, self);

			moveDir = checkMoveInput();
			if (moveDir != Dir4::Invalid && canMoveTo(moveDir))
			{
				// 移動方向決定
				break;
			}

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
		m_scoopContinuous = 0;
		m_cameraOffsetDestination = -moveDir.ToXY() * getToml<double>(U"camera_offset_amount");

		// 移動
		const auto newPos = Vec2(m_pos.actualPos + moveDir.ToXY() * CellPx_24);
		ProcessMoveCharaPos(yield, self, m_pos, newPos, moveDuration());
		// refreshDistField();

		// ギミックチェック
		const auto newPoint = CharaVec2(newPos).MapPoint();
		checkGimmickAt(yield, self, newPoint);

		if (const auto tutorial = PlayScene::Instance().Tutorial())
		{
			tutorial->PlayerService().onMoved(newPos, m_act == PlayerAct::Running);
		}
	}

	bool canMoveTo(Dir4Type dir) const
	{
		if (not CanMoveTo(PlayScene::Instance().GetMap(), m_pos.actualPos, dir)) return false;
		if (const auto tutorial = PlayScene::Instance().Tutorial())
		{
			return tutorial->PlayerService().canMove &&
				tutorial->PlayerService().canMoveTo((m_pos.actualPos + dir.ToXY() * CellPx_24));
		}
		return true;
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
	void focusCameraFor(ActorView self, double scale)
	{
		AnimateEasing<easing>(self, &m_focusCameraRate, scale, 0.5);
	}

	void checkScoopFromMouse(YieldExtended& yield, ActorView self)
	{
		if (RectF(m_pos.actualPos, {CellPx_24, CellPx_24}).intersects(Cursor::PosF()) == false)
			return;

		if (const auto tutorial = PlayScene::Instance().Tutorial())
		{
			if (not tutorial->PlayerService().canScoop) return;
		}

		// 以下、マウスカーソルが当たった状態

		// マウスクリックまで待機
		focusCameraFor(self, getToml<double>(U"focus_scale_large"));
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
				.draw(getToml<ColorF>(U"scoop_rect_color_1"));
		};
		while (true)
		{
			if (checkMoveInput() != Dir4::Invalid && MouseL.pressed() == false) return;
			if (RectF(m_pos.actualPos, {CellPx_24, CellPx_24}).intersects(Cursor::PosF()) == false)
				return;

			yield();
			if (MouseL.pressed()) break;
		}
		AudioAsset(AssetSes::scoop_start).playOneShot();

		// ドラッグ解除まで待機
		m_scoopDrawing = [this]()
		{
			for (int i = 0; i < 4; ++i)
			{
				auto r = RectF(m_pos.actualPos.movedBy(Dir4Type(i).ToXY() * CellPx_24), {CellPx_24, CellPx_24});
				r.draw(getToml<ColorF>(U"scoop_rect_color_2"));
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
				if (const auto tutorial = PlayScene::Instance().Tutorial())
				{
					if (not tutorial->PlayerService().canScoopTo(checkingPos)) continue;
				}

				// 以下、移動させる処理を実行
				// m_distField.Clear();
				m_scoopDrawing = {};
				succeedScoop(yield, self, checkingPos);

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

	void succeedScoop(YieldExtended& yield, ActorView self, const Vector2D<double> checkingPos)
	{
		AudioAsset(AssetSes::scoop_move).playOneShot();

		if (const auto tutorial = PlayScene::Instance().Tutorial())
		{
			tutorial->PlayerService().onScooped(m_pos.actualPos);
		}

		// 上手くすくって別の場所に移動するときの処理
		m_immortal.immortalStock++;
		const double animDuration = getToml<double>(U"scoop_move_duration");
		AnimateEasing<BoomerangParabola>(self, &m_animOffset, Vec2{0, -32}, animDuration);
		ProcessMoveCharaPos(yield, self, m_pos, checkingPos, animDuration);
		refreshDistField();
		m_immortal.immortalStock--;

		if (PlayScene::Instance().GetMap().At(m_pos.actualPos.MapPoint()).kind == TerrainKind::Wall)
		{
			// ペナルティ発生
			RelayTimeDamageAmount(m_pos, GetPlayerScoopedPenaltyDamage(m_scoopContinuous), false);
			m_scoopContinuous++;
		}
	}

	void refreshDistField()
	{
		// ディスタンスフィールドの更新
		const int maxDist = m_faintStealthTime > 0 ? 2 : 11;
		m_distField.Refresh(PlayScene::Instance().GetMap(), m_pos.actualPos, maxDist);
	}

	void checkGimmickAt(YieldExtended& yield, ActorView self, const Point newPoint)
	{
		const auto storedAct = m_act;
		m_act = PlayerAct::Idle;

		auto&& gimmickGrid = PlayScene::Instance().GetGimmick();
		switch (gimmickGrid[newPoint])
		{
		case GimmickKind::Stairs: {
			// ゴール到達
			AudioAsset(AssetSes::stairs_step).playOneShot();
			m_cameraOffsetDestination = {0, 0};
			m_immortal.immortalStock++;
			PlayScene::Instance().GetTimeLimiter().SetImmortal(true);
			PlayScene::Instance().EndTransition();
			yield.WaitForDead(
				AnimateEasing<EaseInBack>(self, &m_cameraScale, 8.0, 0.5));
			AudioAsset(AssetSes::stairs_close).playOneShot();
			yield.WaitForDead(
				AnimateEasing<EaseOutCirc>(self, &m_cameraScale, 10.0, 0.5));
			m_terminated = true;
			break;
		}
		case GimmickKind::Item_Wing: [[fallthrough]];
		case GimmickKind::Item_Helmet: [[fallthrough]];
		case GimmickKind::Item_Pin: [[fallthrough]];
		case GimmickKind::Item_Mine: [[fallthrough]];
		case GimmickKind::Item_LightBulb: [[fallthrough]];
		case GimmickKind::Item_Magnet: [[fallthrough]];
		case GimmickKind::Item_Bookmark: [[fallthrough]];
		case GimmickKind::Item_Explorer: [[fallthrough]];
		case GimmickKind::Item_Grave: [[fallthrough]];
		case GimmickKind::Item_Sun: [[fallthrough]];
		case GimmickKind::Item_Tube: [[fallthrough]];
		case GimmickKind::Item_Solt:
			obtainItemAt(newPoint, gimmickGrid);
			break;
		case GimmickKind::SemiItem_Hourglass: {
			AudioAsset(AssetSes::recover_small).playOneShot();
			RelayTimeHealAmount(m_pos, 10);
			gimmickGrid[newPoint] = GimmickKind::None;
			break;
		}
		case GimmickKind::SemiItem_Vessel: {
			AudioAsset(AssetSes::take_item).playOneShot();
			focusCameraFor<EaseOutBack>(self, getToml<double>(U"focus_scale_large"));
			PlayScene::Instance().RequestHitstopping(0.3);
			yield.WaitForTime(0.3);
			AudioAsset(AssetSes::recover_large).playOneShot();
			focusCameraFor<EaseOutBack>(self, 1.0);
			PlayScene::Instance().GetTimeLimiter().ExtendMax(30.0);
			gimmickGrid[newPoint] = GimmickKind::None;
			break;
		}
		case GimmickKind::Arrow_right: [[fallthrough]];
		case GimmickKind::Arrow_up: [[fallthrough]];
		case GimmickKind::Arrow_left: [[fallthrough]];
		case GimmickKind::Arrow_down:
			m_immortal.immortalStock++;
			moveArrowWarp(yield, self, newPoint);
			m_immortal.immortalStock--;
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
			AudioAsset(AssetSes::take_item).playOneShot();
			m_personal.items[i] = GimmickToItem(gimmickGrid[point]);
			assert(m_personal.items[i] != ConsumableItem::None);
			gimmickGrid[point] = GimmickKind::None;
			break;
		}
	}

	void moveArrowWarp(YieldExtended& yield, ActorView self, const Point point)
	{
		AudioAsset(AssetSes::arrow_step).playOneShot();
		const auto nextPoint =
			GetArrowWarpPoint(PlayScene::Instance().GetMap(), PlayScene::Instance().GetGimmick(), point);
		ProcessArrowWarpCharaPos(yield, self, m_pos, m_animOffset, nextPoint * CellPx_24);
		refreshDistField();
	}

	bool canInstallGimmickNow() const
	{
		if (m_act != PlayerAct::Idle) return false;
		const auto p = m_pos.actualPos.MapPoint();
		auto&& gimmick = PlayScene::Instance().GetGimmick();
		if (gimmick.inBounds(p) == false) return false;
		if (gimmick[p] != GimmickKind::None) return false;
		return true;
	}

	bool gotoStairsByWing(ActorView self)
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

	void Player::Init(const PlayerPersonalData& data, const Vec2& initialPos)
	{
		p_impl->m_personal = data;

		p_impl->m_pos.SetPos(initialPos);

		p_impl->m_distField.Resize(PlayScene::Instance().GetMap().Data().size());

		if (not PlayScene::Instance().Tutorial()) p_impl->PerformInitialCamera(*this);

		p_impl->m_immortal.immortalTime = getToml<double>(U"initial_immortal");

		p_impl->StartFlowchart(*this);

#if _DEBUG
		// p_impl->m_personal.items[0] = ConsumableItem::Pin;
		// p_impl->m_personal.items[1] = ConsumableItem::Sun;
		// p_impl->m_personal.items[2] = ConsumableItem::Mine;
#endif
	}

	void Player::Update()
	{
		p_impl->CheckGameOver(*this);
		ActorBase::Update();
		p_impl->Update();
	}

	double Player::OrderPriority() const
	{
		return CharaOrderPriority(p_impl->m_pos);
	}

	void Player::SendEnemyCollide(const RectF& rect, EnemyKind enemy)
	{
		p_impl->EnemyCollide(*this, rect, enemy);
	}

	bool Player::RequestUseItem(int itemIndex)
	{
		const bool used = p_impl->UseItem(*this, p_impl->m_personal.items[itemIndex]);
		if (used)
		{
			AudioAsset(AssetSes::item_use).playOneShot();
			p_impl->m_personal.items[itemIndex] = ConsumableItem::None;
		}
		return used;
	}

	void Player::PerformTutorialOpening()
	{
		p_impl->PerformTutorialOpening(*this);
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

	bool Player::IsImmortal() const
	{
		return p_impl->m_immortal.IsImmortal();
	}

	bool Player::IsTerminated() const
	{
		return p_impl->m_terminated;
	}

	const PlayerVisionState& Player::Vision() const
	{
		return p_impl->m_vision;
	}
}
