#pragma once

namespace AssetFonts
{
	constexpr StringView F24 = U"F24";
	constexpr StringView F48 = U"F48";

	inline void RegisterAll()
	{
		FontAsset::Register(F24, 24);
		FontAsset::Register(F48, 48);
	}
}
