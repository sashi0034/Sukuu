#pragma once
#include "Play/Forward.h"
#include "AssetsGenerated.h"
#include "Play/Chara/CharaUtil.h"
#include "Util/Dir4.h"
#include "Util/TomlParametersWrapper.h"

namespace Play
{
	TextureRegion GetUsualPlayerTexture(
		const Rect& playerRect, Dir4Type direction, AnimTimer animTimer, bool isWalking);

	TextureRegion GetDeadPlayerTexture(const Rect& playerRect);

	void AnimatePlayerDie(YieldExtended& yield, ActorBase& self, Vec2& m_animOffset);

	void AnimatePlayerUsingWing(YieldExtended& yield, ActorBase& self, Vec2& animOffset, CharaPosition& pos);
}
