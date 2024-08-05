#pragma once

#include "Util/ActorBase.h"
#include "Util/Utilities.h"

namespace Play
{
	using namespace Util;

	constexpr int CellPx_24 = 24;

	constexpr int CharacterPadding_4 = 4;

	constexpr Point GetCharacterCellPadding(const Size& spriteSize)
	{
		return {(CellPx_24 - spriteSize.x) / 2, CellPx_24 - spriteSize.y - CharacterPadding_4};
	}

	constexpr Point GetItemCellPadding(const Size& spriteSize)
	{
		return {(CellPx_24 - spriteSize.x) / 2, (CellPx_24 - spriteSize.y) / 2 - CharacterPadding_4};
	}

	constexpr int MaxItemPossession = 9;

	constexpr double FgEffectPriority = 32767;
	constexpr double BgEffectPriority = -32768;

	constexpr double DefaultCameraScale = 3.0;

	struct BgmInfo
	{
		AssetNameView music;
		double loopBegin;
		double loopEnd;
	};
}
