#pragma once
#include "Play/Forward.h"
#include "AssetsGenerated.h"
#include "Play/Chara/CharaUtil.h"
#include "Util/Dir4.h"
#include "Util/TomlParametersWrapper.h"

namespace Play
{
	static TextureRegion GetPlayerTexture(
		const Rect& playerRect, Dir4Type direction, AnimTimer animTimer, bool isWalking)
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
