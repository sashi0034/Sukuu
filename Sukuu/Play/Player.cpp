#include "stdafx.h"
#include "Player.h"

#include "PlayScene.h"
#include "PlayBgm.h"
#include "Chara/CharaUtil.h"
#include "Effect/ItemObtainEffect.h"
#include "Gm/GameConfig.h"
#include "Item/ItemGrave.h"
#include "Item/ItemMagnet.h"
#include "Item/ItemMine.h"
#include "Item/ItemPin.h"
#include "Item/ItemSun.h"
#include "Other/PlayPenaltyBonus.h"
#include "Player_detail/PlayerAnimation.h"
#include "Player_detail/PlayerDistField.h"
#include "Player_detail/PlayerInternal.h"
#include "Gm/GameCursor.h"
#include "Gm/GamepadObserver.h"
#include "Util/CoroUtil.h"
#include "Util/Dir4.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	using namespace Play::Player_detail;

	template <typename T>
	T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.player." + key);
	}

	constexpr ColorF arrowColor = ColorF(U"#ffc22b");;
}

struct Play::Player::Impl
{
	CoroWeak m_flowchart{};

	PlayerPersonalData m_personal{};
	CharaPosition m_pos;
	Vec2 m_viewGapOffset{};
	double m_moveSpeed = 1.0;
	double m_cameraScale = DefaultCameraScale;
	double m_focusCameraRate = 1.0;
	ActorWeak m_focusCameraProcess{};
	Vec2 m_cameraOffset{};
	Vec2 m_cameraOffsetDestination{};
	PlayerAct m_act = PlayerAct::Idle;
	AnimTimer m_animTimer{};
	Dir4Type m_direction{Dir4::Down};
	PlayerDistFieldInternal m_distField{};
	bool m_terminated{};
	PlayerImmortality m_immortal{};
	bool m_guardHelmet{};
	Trail m_footTrail{};
	int m_trailStock{};
	std::function<void()> m_subUpdating = {};
	bool m_scoopRequested{};
	bool m_slowMotion{};
	int m_scoopContinuous{};
	PlayerVisionState m_vision{};
	double m_faintStealthTime{};
	double m_scoopNoPenaltyTime{};
	RocketSpark m_rocketSpark{};
	bool m_isGameOver{};

	void Update()
	{
#if _DEBUG
		if (KeyNum1.down()) m_vision.mistRemoval = not m_vision.mistRemoval;
#endif
		UpdatePlayerVision(m_vision, m_act);

		m_immortal.immortalTime = std::max(m_immortal.immortalTime - GetDeltaTime(), 0.0);

		m_animTimer.Tick(GetDeltaTime() * (m_act == PlayerAct::Running ? 2 : 1));

		m_faintStealthTime = std::max(m_faintStealthTime - GetDeltaTime(), 0.0);

		m_scoopNoPenaltyTime = std::max(m_scoopNoPenaltyTime - GetDeltaTime(), 0.0);

		// 描画基準点
		const auto drawingTl = getDrawPos();

		// 軌跡更新
		updateTrail(drawingTl);

		if (m_subUpdating)
		{
			// すくうなどの描画
			m_subUpdating();
		}

		// カメラのオフセットを移動
		const auto cameraDestination = [this]
		{
			if (const auto tutorial = PlayCore::Instance().Tutorial())
				if (const auto overriding = tutorial->PlayerService().overrideCamera) return overriding.value();
			return m_cameraOffsetDestination;
		}();
		m_cameraOffset += (cameraDestination - m_cameraOffset)
			* Scene::DeltaTime()
			* getToml<double>(U"camera_offset_speed");

		// ロケット描画
		if (m_scoopNoPenaltyTime > 0)
		{
			drawRocketSpark(drawingTl);
		}

		const ScopedRenderStates2D rs = getRenderState();

		// 自分自身描画
		(void)getPlayerTexture()
			.draw(drawingTl, ColorF{1.0, getTextureAlpha()});

		// ヘルメット描画
		if (m_guardHelmet)
		{
			TextureAsset(AssetImages::helmet_16x16)(0, 0, 16, 16).drawAt(drawingTl + getHelmetOffset());
		}

		ControlPlayerBgm(m_pos.actualPos, PlayCore::Instance().GetMap());
	}

	void CheckGameOver(ActorView self)
	{
		if (m_isGameOver) return;
		if (PlayCore::Instance().GetTimeLimiter().GetData().remainingTime > 0) return;

		// 以下、ゲームオーバー処理開始
		PlayBgm::Instance().EndPlay();
		AudioAsset(AssetSes::fall_down).playOneShot();
		m_isGameOver = true;
		m_flowchart.Kill();
		m_distField.Clear();
		m_subUpdating = {};
		m_slowMotion = false;
		m_immortal.immortalStock++;
		AnimateEasing<EaseOutCirc>(self, &m_cameraScale, 8.0, 0.5);
		StartCoro(self, [this](YieldExtended yield)
		{
			yield.WaitForTime(1.0);
			yield.WaitForExpire(PlayCore::Instance().PerformGameOver());
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
		AnimateEasing<EaseOutBack>(self, &m_cameraScale, DefaultCameraScale, 0.5);
	}

	void PerformTutorialOpening(ActorView self)
	{
		StartCoro(self, [this, self](YieldExtended yield)
		{
			// チュートリアルの最初
			m_flowchart.Kill();
			m_act = PlayerAct::Dead;
			m_cameraScale = 10.0;
			m_cameraOffset = {0, 1200};
			constexpr double duration = 8.0;
			AnimateEasing<EaseOutCubic>(self, &m_cameraOffset, {0, 0}, duration);
			yield.WaitForExpire(
				AnimateEasing<EaseInOutBack>(self, &m_cameraScale, DefaultCameraScale, duration));
			StartFlowchart(self);
		});
	}

	// エネミーとの衝突判定
	bool EnemyCollide(ActorView self, const RectF& rect, EnemyKind enemy)
	{
		if (m_immortal.IsImmortal()) return false;
		if (m_act == PlayerAct::Dead) return false;
		if (PlayCore::Instance().Tutorial() != nullptr) return false; // チュートリアル中は無敵

		const auto player = RectF{m_pos.actualPos, Point::One() * CellPx_24}.stretched(
			getToml<int>(U"collider_padding"));

		if (rect.intersects(player) == false) return false;
		// 以下、当たった状態

		if (m_guardHelmet)
		{
			// ヘルメットガード発動
			AudioAsset(AssetSes::break_helmet).playOneShot();
			m_guardHelmet = false;
			m_immortal.immortalTime = 1.0;
			EffectHelmetConsume(getDrawPos() + getHelmetOffset(), getPlayerTexture());
			return false;
		}

		AudioAsset(AssetSes::damaged).playOneShot();
		m_act = PlayerAct::Dead;
		breakFlowchart();
		focusCameraFor<EaseOutBack>(self, getToml<double>(U"focus_scale_large"));

		if (enemy == EnemyKind::HandMaster)
		{
			// 下の階へ連れ去られる
			StartCoro(self, [this, self](YieldExtended yield) { startAbduction(yield, self); });
			return true;
		}

		// ペナルティとして時間減らす
		RelayTimeDamageAmount(m_pos, GetEnemyAttackDamage(enemy), true);

		// やられた演出
		StartCoro(self, [this, self](YieldExtended yield)
		{
			AnimatePlayerDie(yield, self, m_viewGapOffset, m_cameraOffset);

			focusCameraFor<EaseInOutBack>(self, 1.0);
			m_immortal.immortalTime = getToml<double>(U"recover_immortal");
			StartFlowchart(self);
		});
		return true;
	}

	bool CanUseItem(ConsumableItem item) const
	{
		if (m_slowMotion) return false;
		if (PlayCore::Instance().GetTimeLimiter().GetData().remainingTime <= 0) return false;

		switch (item)
		{
		case ConsumableItem::None:
			return false;
		case ConsumableItem::Wing:
			return m_act == PlayerAct::Idle;
		case ConsumableItem::Helmet:
			return not m_guardHelmet;
		case ConsumableItem::Pin:
			if (m_act != PlayerAct::Idle) return false;
			return true;
		case ConsumableItem::Mine:
			if (canInstallGimmickNow() == false) return false;
			return true;
		case ConsumableItem::LightBulb:
			if (m_vision.mistRemoval) return false;
			return true;
		case ConsumableItem::Magnet:
			return canInstallGimmickNow();
		case ConsumableItem::Bookmark:
			return PlayCore::Instance().GetMiniMap().CanSpotStairsAndAllItems();
		case ConsumableItem::Explorer:
			return not PlayCore::Instance().GetMiniMap().IsShowEnemies();
		case ConsumableItem::Grave:
			return canInstallGimmickNow();
		case ConsumableItem::Sun:
			return m_act == PlayerAct::Idle;
		case ConsumableItem::Tube:
			return m_act != PlayerAct::Dead;
		case ConsumableItem::Solt:
			if (m_faintStealthTime > 0) return false;
			return true;
		case ConsumableItem::Rocket:
			return m_scoopNoPenaltyTime == 0;
		case ConsumableItem::Max:
			return false;
		default: ;
			assert(false);
			return false;
		}
	}

	void UseItem(ActorView self, ConsumableItem item)
	{
		switch (item)
		{
		case ConsumableItem::None:
			break;
		case ConsumableItem::Wing:
			gotoStairsByWing(self);
			break;
		case ConsumableItem::Helmet:
			m_guardHelmet = true;
			break;
		case ConsumableItem::Pin: {
			auto pin = PlayCore::Instance().AsMainContent().Birth(ItemPin());
			pin.Init(m_pos.actualPos, m_direction);
			break;
		}
		case ConsumableItem::Mine: {
			auto mine = PlayCore::Instance().AsMainContent().Birth(ItemMine());
			mine.Init(m_pos.actualPos);
			break;
		}
		case ConsumableItem::LightBulb:
			UseItemLightBulb(self, m_vision);
			break;
		case ConsumableItem::Magnet: {
			auto magnet = PlayCore::Instance().AsMainContent().Birth(ItemMagnet());
			magnet.Init(m_pos.actualPos);
			break;
		}
		case ConsumableItem::Bookmark:
			PlayCore::Instance().GetMiniMap().SpotStairsAndAllItems();
			break;
		case ConsumableItem::Explorer:
			CheckUseItemExplorer(self);
			break;
		case ConsumableItem::Grave: {
			auto grave = PlayCore::Instance().AsMainContent().Birth(ItemGrave());
			grave.Init(m_pos.actualPos);
			break;
		}
		case ConsumableItem::Sun: {
			auto sun = PlayCore::Instance().AsMainContent().Birth(ItemSun());
			sun.Init(m_pos.actualPos, m_direction);
			break;
		}
		case ConsumableItem::Tube: {
			RelayTimeHealAmount(m_pos, getToml<int>(U"tube_heal_amount"));
			break;
		}
		case ConsumableItem::Solt: {
			m_faintStealthTime += getToml<double>(U"solt_time");
			refreshDistField();
			break;
		}
		case ConsumableItem::Rocket: {
			m_scoopNoPenaltyTime = getRocketTime();
			break;
		}
		case ConsumableItem::Max:
			break;
		default: ;
		}
	}

	Mat3x2 CameraTransform() const
	{
		const auto cameraOffset = m_cameraOffset + getToml<Point>(U"default_camera_offset");
		return Mat3x2::Translate({Scene::Center()})
		       .translated(-m_pos.viewPos + cameraOffset - Vec2::One() * CellPx_24 / 2)
		       .scaled(m_cameraScale * m_focusCameraRate, Scene::Center());
	}

private:
	Vec2 getDrawPos() const
	{
		return m_pos.viewPos.movedBy(GetCharacterCellPadding(PlayerCellRect.size) + m_viewGapOffset);
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

	static double getRocketTime()
	{
		return getToml<double>(U"rocket_time");
	}

	void drawRocketSpark(const Vec2& drawingTl)
	{
		const double scaling = 1.0 + 0.1 * Periodic::Sine0_1(1.0s, m_scoopNoPenaltyTime);

		const auto drawCenter = drawingTl.movedBy(Vec2::One() * PlayerCellRect.size / 2);
		const auto size = getToml<double>(U"rocket_size");
		Circle(drawCenter, size * 2.0)
			.drawArc(0_deg, 360_deg * (m_scoopNoPenaltyTime / getRocketTime()), 0, 0.5, RocketSpark::Yellow);

		const ScopedColorAdd2D add{ColorF{0.0, 1.0}};
		m_rocketSpark.Tick(drawCenter, scaling * size);
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

	void updateTrail(const Vec2 drawingTl)
	{
		// 軌跡更新
		// ScopedRenderStates2D rs(BlendState::Additive);
		if (m_trailStock > 0)
		{
			m_footTrail.add(drawingTl.movedBy(PlayerCellRect.bottomCenter()),
			                getToml<ColorF>(U"trail_color"),
			                getToml<int>(U"trail_size"));
		}
		m_footTrail.update(GetDeltaTime());
		m_footTrail.draw();
	}

	void breakFlowchart()
	{
		m_cameraOffsetDestination = {0, 0};
		m_flowchart.Kill();
		m_distField.Clear();
		m_subUpdating = {};
		m_scoopRequested = false;
		m_slowMotion = false;
	}

	static Vec2 getDirectionCameraOffset(Dir4Type moveDir)
	{
		const int c = Gm::GameConfig::Instance().camera_move;
		const int move = getToml<double>(U"camera_move_min") + getToml<double>(U"camera_move_step") * c;
		return -moveDir.ToXY() * move;
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
			// マウス右クリックで向き変更
			checkMouseDirectionRotation(yield);

			// プレイヤーを掬おうとしてるかチェック
			checkScoopProcess(yield, self);

			moveDir = CheckMoveInput();
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
		m_act = IsDashingInput() ? PlayerAct::Running : PlayerAct::Walk;
		m_direction = moveDir;
		m_scoopContinuous = 0;
		m_cameraOffsetDestination = getDirectionCameraOffset(moveDir);

		// 移動
		const auto newPos = Vec2(m_pos.actualPos + moveDir.ToXY() * CellPx_24);
		ProcessMoveCharaPos(yield, self, m_pos, newPos, moveDuration());
		// refreshDistField();

		// ギミックチェック
		const auto newPoint = CharaVec2(newPos).MapPoint();
		checkGimmickAt(yield, self, newPoint);

		if (const auto tutorial = PlayCore::Instance().Tutorial())
		{
			tutorial->PlayerService().onMoved(newPos, m_act == PlayerAct::Running);
		}
	}


	bool canMoveTo(Dir4Type dir) const
	{
		if (not CanMoveTo(PlayCore::Instance().GetMap(), m_pos.actualPos, dir)) return false;
		if (const auto tutorial = PlayCore::Instance().Tutorial())
		{
			return tutorial->PlayerService().canMove &&
				tutorial->PlayerService().canMoveTo((m_pos.actualPos + dir.ToXY() * CellPx_24));
		}
		return true;
	}

	template <double easing(double) = EaseInOutSine>
	void focusCameraFor(ActorView self, double scale)
	{
		m_focusCameraProcess.Kill();
		m_focusCameraProcess = AnimateEasing<easing>(self, &m_focusCameraRate, scale, 0.5);
	}

	static void drawArrow(const Vec2& center, Dir4Type dir, double length)
	{
		(void)Shape2D::Arrow(Line{center, center + dir.ToXY() * length}, 20, Vec2::One() * 16)
		      .draw(ColorF(arrowColor, 0.9)).drawFrame(0.5, ColorF(arrowColor * 0.3, 0.9));
	}

	void checkMouseDirectionRotation(YieldExtended& yield)
	{
		const auto pressed = []()
		{
			return Gm::IsUsingGamepad()
				       ? IsGamepadPressed(Gm::GamepadButton::LT)
				       : MouseR.pressed();
		};

		if (not pressed()) return;

		// 方向転換をチェック
		while (true)
		{
			yield();
			if (not pressed())
			{
				m_subUpdating = {};
				return;
			}

			const auto center = m_pos.actualPos.movedBy(Vec2::One() * CellPx_24 / 2);
			auto moveInput = CheckMoveInput();
			m_direction = Gm::IsUsingGamepad()
				              ? (moveInput != Dir4::Invalid ? moveInput : m_direction)
				              : Dir4::FromXY(Cursor::PosF() - center);
			m_cameraOffsetDestination = getDirectionCameraOffset(m_direction);
			m_subUpdating = [center, d = m_direction, t = m_animTimer.Time()]()
			{
				drawArrow(center, d, 32 - 4 * Periodic::Jump0_1(0.5s, t));
				if (not Gm::IsUsingGamepad()) Circle(Cursor::PosF(), 16).draw(ColorF{arrowColor, 0.5});
			};
		}
	}

	void checkScoopProcess(YieldExtended& yield, ActorView self)
	{
		if (const auto tutorial = PlayCore::Instance().Tutorial())
		{
			if (not tutorial->PlayerService().canScoop) return;
		}

		// マウスクリックまで待機
		m_subUpdating = [this]
		{
			const auto playerRect = RectF(m_pos.actualPos, {CellPx_24, CellPx_24});
			const bool intersectsCursor =
				not Gm::IsUsingGamepad() && playerRect.intersects(GetCursorRect());

			if (not m_scoopRequested)
				m_scoopRequested = CheckScoopRequestInput(intersectsCursor);
			const bool isAttempt = not m_scoopRequested && IsScoopAttemptInput(intersectsCursor);

			const auto cellColor = m_scoopRequested
				                       ? getToml<ColorF>(U"scoop_rect_color_2")
				                       : getToml<ColorF>(U"scoop_rect_color_1");

			const bool isDrawCell = isAttempt || m_scoopRequested;
			if (not isDrawCell) return;

			// セル描画
			constexpr double polygoneThickness = 0.5;
			Polygon polygon = playerRect.asPolygon();
			if (intersectsCursor)
			{
				// カーソルを代替表示
				Gm::RequestHideGameCursor();
				const auto cursorCircle = Circle(Cursor::PosF(), GetCursorSize() / 2);
				if (not polygon.append(cursorCircle.asPolygon()))
				{
					cursorCircle.draw(cellColor)
					            .drawFrame(polygoneThickness, ColorF(cellColor.rgb() * 0.5, 1.0));
				}
			}
			(void)polygon //.movedBy(m_pos.viewPos - m_pos.actualPos)
			      .draw(cellColor)
			      .drawFrame(polygoneThickness, ColorF(cellColor.rgb() * 0.5, 1.0));
		};

		// すくうが要求されるまで処理を進めない
		if (not m_scoopRequested) return;
		AudioAsset(AssetSes::scoop_start).playOneShot();

		// すくう方向を決定
		double scoopingCharge{};
		Dir4Type scoopingDir{Dir4::Invalid};
		focusCameraFor(self, getToml<double>(U"focus_scale_large"));
		m_subUpdating = [this, &scoopingCharge, &scoopingDir]()
		{
			const auto cellColor = getToml<ColorF>(U"scoop_rect_color_2");
			for (int i = 0; i < 4; ++i)
			{
				// 4方向セル
				if (i == scoopingDir) continue;
				auto r = RectF(m_pos.actualPos.movedBy(Dir4Type(i).ToXY() * CellPx_24), {CellPx_24, CellPx_24});
				(void)r.draw(cellColor).drawFrame(0.5, ColorF(cellColor.rgb() * 0.5, 1.0));
			}
			if (Gm::IsUsingGamepad() && scoopingCharge > 0)
			{
				// 矢印
				const double arrowLength = 40 * (scoopingCharge);
				drawArrow(m_pos.actualPos.movedBy(Vec2::One() * CellPx_24 / 2) + scoopingDir.ToXY() * CellPx_24 / 2,
				          scoopingDir,
				          arrowLength);
			}
		};
		m_slowMotion = true;
		while (true)
		{
			yield();
			if (IsScoopCancelInput())
			{
				// すくう解除
				focusCameraFor(self, 1.0);
				m_subUpdating = {};
				m_scoopRequested = false;
				m_slowMotion = false;
				break;
			}

			// すくう方向を決定
			scoopingDir = CheckScoopMoveInput(m_pos.actualPos);
			if (scoopingDir == Dir4::Invalid)
			{
				scoopingCharge = 0;
				continue;
			}

			const auto nextPos = m_pos.actualPos.movedBy(Dir4Type(scoopingDir).ToXY() * CellPx_24);
			if (const auto tutorial = PlayCore::Instance().Tutorial())
			{
				if (not tutorial->PlayerService().canScoopTo(nextPos))
				{
					scoopingCharge = 0;
					continue;
				}
			}

			if (Gm::IsUsingGamepad() && scoopingCharge < 1.0)
			{
				// ゲームパッドのときは、ためが必要
				constexpr double chargingTIme = 0.15;
				scoopingCharge = Math::Min(1.0, scoopingCharge + Scene::DeltaTime() / chargingTIme);
				continue;
			}

			// 以下、移動させる処理を実行
			// m_distField.Clear();
			m_scoopRequested = false;
			m_subUpdating = {};
			m_slowMotion = false;
			succeedScoop(yield, self, nextPos);
			focusCameraFor(self, 1.0);
			break;
		}
	}

	void succeedScoop(YieldExtended& yield, ActorView self, const Vector2D<double> checkingPos)
	{
		AudioAsset(AssetSes::scoop_move).playOneShot();

		if (const auto tutorial = PlayCore::Instance().Tutorial())
		{
			tutorial->PlayerService().onScooped(m_pos.actualPos);
		}

		// 上手くすくって別の場所に移動するときの処理
		m_immortal.immortalStock++;
		m_trailStock++;

		const double animDuration = getToml<double>(U"scoop_move_duration");
		AnimateEasing<BoomerangParabola>(self, &m_viewGapOffset, Vec2{0, -32}, animDuration);
		ProcessMoveCharaPos(yield, self, m_pos, checkingPos, animDuration);
		refreshDistField();

		m_trailStock--;
		m_immortal.immortalStock--;

		if (PlayCore::Instance().GetMap().At(m_pos.actualPos.MapPoint()).kind == TerrainKind::Wall
			&& m_scoopNoPenaltyTime <= 0)
		{
			// ペナルティ発生
			RelayTimeDamageAmount(m_pos, static_cast<int>(GetPlayerScoopedPenaltyDamage(m_scoopContinuous)), false);
			m_scoopContinuous++;
		}
	}

	void refreshDistField()
	{
		// ディスタンスフィールドの更新
		const int maxDist = m_faintStealthTime > 0 ? 2 : 11;
		m_distField.Refresh(PlayCore::Instance().GetMap(), m_pos.actualPos, maxDist);
	}

	void prepareEndFloor()
	{
		m_cameraOffsetDestination = {0, 0};
		m_subUpdating = {};
		m_immortal.immortalStock++;
		PlayCore::Instance().GetTimeLimiter().SetImmortal(true);
	}

	void checkGimmickAt(YieldExtended& yield, ActorView self, const Point newPoint)
	{
		const auto storedAct = m_act;
		m_act = PlayerAct::Idle;

		auto&& gimmickGrid = PlayCore::Instance().GetGimmick();
		switch (gimmickGrid[newPoint])
		{
		case GimmickKind::Stairs: {
			// ゴール到達
			AudioAsset(AssetSes::stairs_step).playOneShot();
			prepareEndFloor();
			PlayCore::Instance().EndTransition();
			yield.WaitForExpire(
				AnimateEasing<EaseInBack>(self, &m_cameraScale, 8.0, 0.5));
			AudioAsset(AssetSes::stairs_close).playOneShot();
			yield.WaitForExpire(
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
		case GimmickKind::Item_Solt: [[fallthrough]];
		case GimmickKind::Item_Rocket:
			obtainItemAt(newPoint, gimmickGrid);
			break;
		case GimmickKind::SemiItem_Hourglass: {
			AudioAsset(AssetSes::recover_small).playOneShot();
			RelayTimeHealAmount(m_pos, 15);
			gimmickGrid[newPoint] = GimmickKind::None;
			break;
		}
		case GimmickKind::SemiItem_Vessel: {
			AudioAsset(AssetSes::take_item).playOneShot();
			focusCameraFor<EaseOutBack>(self, getToml<double>(U"focus_scale_large"));
			itemObtainEffect();
			PlayCore::Instance().RequestHitstopping(0.3);
			yield.WaitForTime(0.3);
			AudioAsset(AssetSes::recover_large).playOneShot();
			focusCameraFor<EaseOutBack>(self, 1.0);
			PlayCore::Instance().GetTimeLimiter().ExtendMax(30.0);
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
			itemObtainEffect();
			m_personal.items[i] = GimmickToItem(gimmickGrid[point]);
			assert(m_personal.items[i] != ConsumableItem::None);
			gimmickGrid[point] = GimmickKind::None;
			break;
		}
	}

	void itemObtainEffect() const
	{
		PlayCore::Instance().BgEffect().add(MakeItemObtainEffect(m_pos.viewPos.movedBy(CellPx_24 / 2, CellPx_24 / 2)));
	}

	void moveArrowWarp(YieldExtended& yield, ActorView self, const Point point)
	{
		AudioAsset(AssetSes::arrow_step).playOneShot();
		const auto nextPoint =
			GetArrowWarpPoint(PlayCore::Instance().GetMap(), PlayCore::Instance().GetGimmick(), point);
		ProcessArrowWarpCharaPos(yield, self, m_pos, m_viewGapOffset, nextPoint * CellPx_24);
		refreshDistField();
	}

	bool canInstallGimmickNow() const
	{
		if (m_act != PlayerAct::Idle) return false;
		const auto p = m_pos.actualPos.MapPoint();
		auto&& gimmick = PlayCore::Instance().GetGimmick();
		if (gimmick.inBounds(p) == false) return false;
		if (gimmick[p] != GimmickKind::None) return false;
		return true;
	}

	void gotoStairsByWing(ActorView self)
	{
		breakFlowchart();
		StartCoro(self, [this, self](YieldExtended yield) mutable
		{
			AnimatePlayerUsingWing(yield, self, m_viewGapOffset, m_pos);
			StartFlowchart(self);
		});
	}

	void startAbduction(YieldExtended& yield, ActorView self)
	{
		// 連れ去られる
		prepareEndFloor();
		m_act = PlayerAct::Idle;
		m_direction = Dir4::Up;

		PlayCore::Instance().RequestHitstopping(0.5);
		yield.WaitForTime(0.5, Scene::DeltaTime);

		yield.WaitForExpire(
			AnimateEasing<EaseOutCirc>(self, &m_viewGapOffset, Vec2{0, -64}, 0.3));
		AnimateEasing<EaseInCirc>(self, &m_viewGapOffset, Vec2{0, getToml<int>(U"abduction_y")}, 1.0);
		yield.WaitForTime(0.5);
		PlayCore::Instance().EndTransition();
		yield.WaitForTime(getToml<double>(U"abduction_intermission"));
		m_terminated = true;
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

		p_impl->m_distField.Resize(PlayCore::Instance().GetMap().Data().size());

		if (not PlayCore::Instance().Tutorial()) p_impl->PerformInitialCamera(*this);

		p_impl->m_immortal.immortalTime = getToml<double>(U"initial_immortal");

		p_impl->StartFlowchart(*this);

#if _DEBUG
		p_impl->m_personal.items[0] = ConsumableItem::Rocket;
		p_impl->m_personal.items[1] = ConsumableItem::Wing;
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

	bool Player::SendEnemyCollide(const RectF& rect, EnemyKind enemy)
	{
		return p_impl->EnemyCollide(*this, rect, enemy);
	}

	bool Player::CanUseItem(int itemIndex) const
	{
		return p_impl->CanUseItem(p_impl->m_personal.items[itemIndex]);
	}

	void Player::RequestUseItem(int itemIndex)
	{
		p_impl->UseItem(*this, p_impl->m_personal.items[itemIndex]);
		AudioAsset(AssetSes::item_use).playOneShot();
		p_impl->m_personal.items[itemIndex] = ConsumableItem::None;
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

	Vec2 Player::GetActualViewPos() const
	{
		return p_impl->m_pos.viewPos + p_impl->m_viewGapOffset;
	}

	const PlayerDistField& Player::DistField() const
	{
		return p_impl->m_distField.Field();
	}

	bool Player::IsSlowMotion() const
	{
		return p_impl->m_slowMotion;
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
