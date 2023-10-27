#pragma once
#include "Play/Forward.h"
#include "AssetsGenerated.h"
#include "Play/Chara/CharaUtil.h"
#include "Util/Dir4.h"
#include "Util/TomlParametersWrapper.h"

namespace Play
{
	TextureRegion GetUsualPlayerTexture(
		Dir4Type direction, AnimTimer animTimer, bool isWalking);

	TextureRegion GetDeadPlayerTexture();

	void AnimatePlayerDie(YieldExtended& yield, ActorView self, Vec2& animOffset, Vec2& cameraOffset);

	void AnimatePlayerUsingWing(YieldExtended& yield, ActorBase& self, Vec2& animOffset, CharaPosition& pos);
}
