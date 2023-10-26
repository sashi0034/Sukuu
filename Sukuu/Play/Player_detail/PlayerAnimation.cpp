#pragma once

#include "PlayerAnimation.h"

#include "Play/PlayScene.h"
#include "Util/EasingAnimation.h"

namespace Play
{
	TextureRegion GetUsualPlayerTexture(
		const Rect& playerRect, Dir4Type direction, AnimTimer animTimer, bool isWalking)
	{
		{
			auto&& sheet = TextureAsset(AssetImages::beatrice_32x32);
			const int animInterval = GetTomlParameter<int>(U"play.player.anim_interval");

			if (isWalking)
			{
				switch (direction.GetIndex())
				{
				case Dir4Type::Right:
					return sheet(playerRect.movedBy(
						playerRect.w * animTimer.SliceFrames(animInterval, 6),
						playerRect.h * 4));
				case Dir4Type::Up:
					return sheet(playerRect.movedBy(
						playerRect.w * animTimer.SliceFrames(animInterval, 4), playerRect.h * 5));
				case Dir4Type::Left:
					return sheet(playerRect.movedBy(
						playerRect.w * animTimer.SliceFrames(animInterval, 6),
						playerRect.h * 4)).mirrored();
				case Dir4Type::Down:
					return sheet(playerRect.movedBy(
						playerRect.w * animTimer.SliceFrames(animInterval, 4),
						playerRect.h * 3));
				default:
					return {};
				}
			}

			switch (direction.GetIndex())
			{
			case Dir4Type::Right:
				return sheet(playerRect.movedBy(
					playerRect.w * animTimer.SliceFrames(animInterval, {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 4}),
					playerRect.h));
			case Dir4Type::Up:
				return sheet(playerRect.movedBy(
					playerRect.w * animTimer.SliceFrames(animInterval, 4), playerRect.h * 2));
			case Dir4Type::Left:
				return sheet(playerRect.movedBy(
						playerRect.w * animTimer.SliceFrames(animInterval, {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 4}),
						playerRect.h))
					.mirrored();
			case Dir4Type::Down:
				return sheet(playerRect.movedBy(
					playerRect.w * animTimer.SliceFrames(animInterval, {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 4}), 0));
			default:
				return {};
			}
		}
	}

	TextureRegion GetDeadPlayerTexture(const Rect& playerRect)
	{
		{
			auto&& sheet = TextureAsset(AssetImages::beatrice_32x32);
			return sheet(playerRect.movedBy(0, playerRect.h * 6));
		}
	}

	void AnimatePlayerDie(YieldExtended& yield, ActorBase& self, Vec2& animOffset, Vec2& cameraOffset)
	{
		SetTimeScale(GetTomlParameter<double>(U"play.player.hitstopping_timescale"));
		yield.WaitForTime(0.5, Scene::DeltaTime);
		SetTimeScale(1);

		// カメラシェイク
		StartCoro(self, [self, &cameraOffset](YieldExtended yield1) mutable
		{
			yield1.WaitForDead(AnimateEasing<EaseOutBack>(self, &cameraOffset, Vec2{-40, 0}, 0.1));
			yield1.WaitForDead(AnimateEasing<EaseOutBack>(self, &cameraOffset, Vec2{20, 0}, 0.1));
			yield1.WaitForDead(AnimateEasing<EaseOutBack>(self, &cameraOffset, Vec2{0, 0}, 0.1));
		});

		yield.WaitForDead(AnimateEasing<BoomerangParabola>(
				self,
				&animOffset,
				GetTomlParameter<Vec2>(U"play.player.dead_animation_offset_1"),
				GetTomlParameter<double>(U"play.player.dead_animation_duration_1"))
		);
		yield.WaitForDead(AnimateEasing<BoomerangParabola>(
			self,
			&animOffset,
			GetTomlParameter<Vec2>(U"play.player.dead_animation_offset_2"),
			GetTomlParameter<double>(U"play.player.dead_animation_duration_2")));
		yield.WaitForTime(GetTomlParameter<double>(U"play.player.dead_pause_duration"));
	}

	void AnimatePlayerUsingWing(YieldExtended& yield, ActorBase& self, Vec2& animOffset, CharaPosition& pos)
	{
		const auto stairs = PlayScene::Instance().GetGimmick().GetSinglePoint(GimmickKind::Stairs);
		const Vec2 warpPos = stairs.movedBy(0, 1) * CellPx_24;
		const double animDuration = GetTomlParameter<double>(U"play.player.warp_duration");
		AnimateEasing<BoomerangParabola>(
			self, &animOffset, GetTomlParameter<Vec2>(U"play.player.warp_jump_offset"), animDuration);
		ProcessMoveCharaPos<EaseInOutBack>(
			yield, self, pos, warpPos, animDuration);
	}
}
