#pragma once

namespace AssetKeys
{
	constexpr AssetNameView RocknRoll24 = U"RocknRoll24";
	constexpr AssetNameView RocknRoll24Msdf = U"RocknRoll24_MSDF";

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
		FontAsset::Register(RocknRoll24Msdf, FontMethod::MSDF, 24, U"asset/font/RocknRoll/RocknRollOne-Regular.ttf");

		TextureAsset::Register(U"⏳", U"⏳"_emoji);
		TextureAsset::Register(U"😎", U"😎"_emoji);
		TextureAsset::Register(U"🤔", U"🤔"_emoji);
		TextureAsset::Register(U"😴", U"😴"_emoji);
		TextureAsset::Register(U"😬", U"😬"_emoji);

		TextureAsset::Register(U"🪶", U"🪶"_emoji);
		TextureAsset::Register(U"⛑️", U"⛑️"_emoji);
		TextureAsset::Register(U"📌", U"📌"_emoji);
		TextureAsset::Register(U"💣", U"💣"_emoji);
		TextureAsset::Register(U"💡", U"💡"_emoji);
		TextureAsset::Register(U"🧲", U"🧲"_emoji);
		TextureAsset::Register(U"🔖", U"🔖"_emoji);
		TextureAsset::Register(U"🗂️", U"🗂️"_emoji);
		TextureAsset::Register(U"🪦", U"🪦"_emoji);
		TextureAsset::Register(U"🌞", U"🌞"_emoji);
		TextureAsset::Register(U"🧪", U"🧪"_emoji);
		TextureAsset::Register(U"🧂", U"🧂"_emoji);

		RegisterShader();
	}
}
