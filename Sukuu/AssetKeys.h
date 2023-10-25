#pragma once

namespace AssetKeys
{
	constexpr AssetNameView F24 = U"F24";
	constexpr AssetNameView F48 = U"F48";

	constexpr AssetNameView PsCaveVision = U"asset/shader/cave_vision";
	constexpr AssetNameView VsCaveVision = U"asset/shader/soft_shape";

	inline void RegisterShader()
	{
		PixelShaderAsset::Register(PsCaveVision, HLSL(PsCaveVision + U".hlsl"));
		VertexShaderAsset::Register(VsCaveVision, HLSL(VsCaveVision + U".hlsl"));
	}

	inline void RegisterAll()
	{
		FontAsset::Register(F24, 24);
		FontAsset::Register(F48, 48);

		TextureAsset::Register(U"😎", U"😎"_emoji);
		TextureAsset::Register(U"🤔", U"🤔"_emoji);
		TextureAsset::Register(U"🪶", U"🪶"_emoji);
		TextureAsset::Register(U"⛑️", U"⛑️"_emoji);
		TextureAsset::Register(U"📌", U"📌"_emoji);

		RegisterShader();
	}
}
