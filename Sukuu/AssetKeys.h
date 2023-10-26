#pragma once

namespace AssetKeys
{
	constexpr AssetNameView RocknRoll24 = U"RocknRoll24";
	constexpr AssetNameView RocknRoll48 = U"RocknRoll48";

	constexpr AssetNameView PsCaveVision = U"asset/shader/cave_vision";
	constexpr AssetNameView VsCaveVision = U"asset/shader/soft_shape";

	inline void RegisterShader()
	{
		PixelShaderAsset::Register(PsCaveVision, HLSL(PsCaveVision + U".hlsl"));
		VertexShaderAsset::Register(VsCaveVision, HLSL(VsCaveVision + U".hlsl"));
	}

	inline void RegisterAll()
	{
		FontAsset::Register(RocknRoll24, 24, U"asset/font/RocknRoll/RocknRollOne-Regular.ttf");
		FontAsset::Register(RocknRoll48, 48, U"asset/font/RocknRoll/RocknRollOne-Regular.ttf");

		TextureAsset::Register(U"⏳", U"⏳"_emoji);
		TextureAsset::Register(U"😎", U"😎"_emoji);
		TextureAsset::Register(U"🤔", U"🤔"_emoji);
		TextureAsset::Register(U"😴", U"😴"_emoji);
		TextureAsset::Register(U"🪶", U"🪶"_emoji);
		TextureAsset::Register(U"⛑️", U"⛑️"_emoji);
		TextureAsset::Register(U"📌", U"📌"_emoji);

		RegisterShader();
	}
}
