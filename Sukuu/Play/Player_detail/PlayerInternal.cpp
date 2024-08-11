#include "stdafx.h"
#include "PlayerInternal.h"

#include "Gm/GamepadObserver.h"
#include "Play/PlayBgm.h"
#include "Play/PlayCore.h"
#include "Util/CoroUtil.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.player." + key);
	}
}

namespace Play::Player_detail
{
	bool IsDashingInput()
	{
		const bool pressed = Gm::IsUsingGamepad()
			                     ? IsGamepadPressed(Gm::GamepadButton::B)
			                     : KeyShift.pressed();
		return pressed || PlayCore::Instance().GetDashKeep().IsKeeping();
	}

	Dir4Type CheckMoveInput()
	{
		if (Gm::IsUsingGamepad())
		{
			if (IsGamepadPressed(Gm::GamepadButton::DUp)) return Dir4::Up;
			if (IsGamepadPressed(Gm::GamepadButton::DDown)) return Dir4::Down;
			if (IsGamepadPressed(Gm::GamepadButton::DLeft)) return Dir4::Left;
			if (IsGamepadPressed(Gm::GamepadButton::DRight)) return Dir4::Right;
		}
		else
		{
			if (KeyW.pressed() || KeyUp.pressed()) return Dir4::Up;
			if (KeyS.pressed() || KeyDown.pressed()) return Dir4::Down;
			if (KeyA.pressed() || KeyLeft.pressed()) return Dir4::Left;
			if (KeyD.pressed() || KeyRight.pressed()) return Dir4::Right;
		}
		return Dir4::Invalid;
	}

	bool IsScoopAttemptInput(bool intersectsCursor)
	{
		if (Gm::IsUsingGamepad())
		{
			if (IsGamepadPressed(Gm::GamepadButton::RT)) return true;
		}
		else
		{
			if (intersectsCursor) return true;
		}
		return false;
	}

	bool CheckScoopRequestInput(bool intersectsCursor)
	{
		if (Gm::IsUsingGamepad())
		{
			if (IsGamepadUp(Gm::GamepadButton::RT)) return true;
		}
		else
		{
			if (intersectsCursor && MouseL.down()) return true;
		}
		return false;
	}

	bool IsScoopCancelInput()
	{
		if (Gm::IsUsingGamepad())
		{
			return IsGamepadUp(Gm::GamepadButton::RT);
		}
		else
		{
			return MouseL.down();
		}
	}

	Dir4Type CheckScoopMoveInput(const CharaVec2& actualPos)
	{
		if (Gm::IsUsingGamepad())
		{
			return CheckMoveInput();
		}
		else
		{
			// もともとのマスからカーソルが離れたら処理スタート
			const auto centerRect = RectF(actualPos, Vec2{CellPx_24, CellPx_24});
			if (centerRect.intersects(GetCursorRect())) return Dir4::Invalid;

			const auto centerPoint = actualPos.movedBy(Point::One() * CellPx_24 / 2);

			// カーソルをもとに目標の方向を決める
			return Dir4::FromXY(Cursor::PosF() - centerPoint);
		}
	}

	void UseItemLightBulb(ActorView self, PlayerVisionState& vision)
	{
		// 視界クリア
		vision.mistRemoval = true;
		StartCoro(self, [&vision](YieldExtended yield)
		{
			yield.WaitForTime(getToml<double>(U"light_bulb_duration"));
			vision.mistRemoval = false;
		});
	}

	void UseItemExplorer(ActorView self)
	{
		// エネミーを一定時間全表示
		auto&& minimap = PlayCore::Instance().GetMiniMap();
		minimap.SetShowEnemies(true);
		StartCoro(self, [](YieldExtended yield)
		{
			yield.WaitForTime(getToml<double>(U"explorer_enemies_duration"));
			auto&& minimap1 = PlayCore::Instance().GetMiniMap();
			if (minimap1.IsShowEnemies() == false) return;
			minimap1.SetShowEnemies(false);
		});
	}

	void UpdatePlayerVision(PlayerVisionState& vision, PlayerAct act)
	{
		const double d = Scene::DeltaTime() * 5.0;
		double radius{1.0};
		double heartbeat{1.0};
		if (act == PlayerAct::Running)
		{
			radius = 0.9;
			heartbeat = 2.1;
		}
		else if (act == PlayerAct::Walk)
		{
			heartbeat = 1.5;
		}

		vision.radiusRate = Math::Lerp(vision.radiusRate, radius, d);
		vision.heartbeatRate = Math::Lerp(vision.heartbeatRate, heartbeat, d);
	}

	void ControlPlayerBgm(const CharaVec2& pos, const MapGrid& map)
	{
		double rate = 1.0;
		const auto currPoint = pos.MapPoint();
		if (not map.Data().inBounds(currPoint) || map.At(currPoint).kind == TerrainKind::Wall)
		{
			// 壁にいる時は、ボリューム下げる
			rate = 0.3;
		}
		PlayBgm::Instance().SetVolumeRate(rate);
	}

	static void constructRocketSpark(Array<TrailMotion>& trails)
	{
		const double offsets[3] = {Random(1.0, 2.0), Random(1.0, 2.0), Random(1.0, 2.0)};
		for (int i = 0; i < 3; ++i)
		{
			trails <<
				TrailMotion{}
				.setScaleFunction([](double t)
				{
					return (0.5 - AbsDiff(t, 0.5));
				})
				.setPositionFunction([i, &offsets](double t)
				{
					const double x = ((72 + i * 4)) * -Math::Cos(offsets[i] + t * (4.0 + i * (1 + offsets[i])) * 3.0);
					const double y = ((72 + i * 4)) * Math::Sin(offsets[i] + t * (4.0 + i * (1 + offsets[i])) * 2.0);
					return Vec2{x, y};
				})
				.setSizeFunction([](double t)
				{
					return (16.0 + Periodic::Sine0_1(0.2s, t) * 48.0);
				})
				.setColor(ColorF(RocketSpark::Yellow, 1.0))
				.setFrequency(60.0);
		}
	}

	void RocketSpark::Tick(const Vec2& center, double size)
	{
		if (m_trails.isEmpty()) constructRocketSpark(m_trails);

		const Transformer2D transform{
			Mat3x2::Scale(size / 80.0).translated(center).rotated(45_deg, center)
		};
		const ScopedRenderStates2D blend{
			SamplerState::ClampLinear, BlendState::Default2D, RasterizerState::WireframeCullBack
		};

		for (auto& trail : m_trails)
		{
			trail.update(GetDeltaTime());
			trail.draw();
		}
	}
}
