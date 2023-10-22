#pragma once

#include "Assets.h"
#include "Util/ActorBase.h"

namespace Play
{
	using namespace Util;

	constexpr int CellPx_24 = 24;

	constexpr int CharacterPadding_4 = 4;

	inline constexpr Point GetCharacter24Padding(const Size& spriteSize)
	{
		return {(CellPx_24 - spriteSize.x) / 2, CellPx_24 - spriteSize.y - CharacterPadding_4};
	};
}
