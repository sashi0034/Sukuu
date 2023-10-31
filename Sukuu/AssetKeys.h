#pragma once

namespace AssetKeys
{
	constexpr AssetNameView RocknRoll_24_Bitmap = U"RocknRoll_24_Bitmap";
	constexpr AssetNameView RocknRoll_72_Bitmap = U"RocknRoll_72_Bitmap";
	constexpr AssetNameView RocknRoll_Sdf = U"RocknRoll_Sdf";
	constexpr AssetNameView RocknRoll_Sdf_Bold = U"RocknRoll_Sdf_Bold";

	constexpr AssetNameView PsCaveVision = U"asset/shader/cave_vision";
	constexpr AssetNameView PsMultiTextureMask = U"asset/shader/multi_texture_mask";
	constexpr AssetNameView VsCaveVision = U"asset/shader/soft_shape";

	inline void RegisterShader()
	{
		PixelShaderAsset::Register(PsCaveVision, HLSL(PsCaveVision + U".hlsl"));
		PixelShaderAsset::Register(PsMultiTextureMask, HLSL(PsMultiTextureMask + U".hlsl"));
		VertexShaderAsset::Register(VsCaveVision, HLSL(VsCaveVision + U".hlsl"));
	}

	inline void RegisterAll()
	{
		FontAsset::Register(RocknRoll_24_Bitmap, 24, U"asset/font/RocknRoll/RocknRollOne-Regular.ttf");
		FontAsset::Register(RocknRoll_72_Bitmap, 72, U"asset/font/RocknRoll/RocknRollOne-Regular.ttf");
		FontAsset::Register(RocknRoll_Sdf, FontMethod::SDF, 24, U"asset/font/RocknRoll/RocknRollOne-Regular.ttf");
		FontAsset::Register(
			RocknRoll_Sdf_Bold, FontMethod::SDF, 24, U"asset/font/RocknRoll/RocknRollOne-Regular.ttf", FontStyle::Bold);

		TextureAsset::Register(U"👉", U"👉"_emoji);

		TextureAsset::Register(U"⏳", U"⏳"_emoji);
		TextureAsset::Register(U"😎", U"😎"_emoji);
		TextureAsset::Register(U"🤔", U"🤔"_emoji);
		TextureAsset::Register(U"😴", U"😴"_emoji);
		TextureAsset::Register(U"😬", U"😬"_emoji);
		TextureAsset::Register(U"🤗", U"🤗"_emoji);

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
