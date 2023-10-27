#pragma once
#include "Play/Chara/CharaUtil.h"
#include "Util/TomlParametersWrapper.h"

namespace Play::Item_detail
{
	double GetItemCharaOrderPriority(const CharaPosition& pos, const Size& size);

	RectF GetItemCollider(const CharaPosition& pos, const Size& size);
}
