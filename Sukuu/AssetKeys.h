#pragma once

namespace AssetKeys
{
	constexpr AssetNameView RocknRoll_24_Bitmap = U"RocknRoll_24_Bitmap";
	constexpr AssetNameView RocknRoll_72_Bitmap = U"RocknRoll_72_Bitmap";
	constexpr AssetNameView RocknRoll_Sdf = U"RocknRoll_Sdf";
	constexpr AssetNameView RocknRoll_Sdf_Bold = U"RocknRoll_Sdf_Bold";

	constexpr AssetNameView gamepad = U"asset/other/gamepad.png";

	constexpr AssetNameView PsCaveVision = U"asset/shader/cave_vision";
	constexpr AssetNameView PsMultiTextureMask = U"asset/shader/multi_texture_mask";
	constexpr AssetNameView PsRasterScroll = U"asset/shader/raster_scroll";
	constexpr AssetNameView PsRgbToBgr = U"asset/shader/rgb_to_bgr";
	constexpr AssetNameView PsGrayscale = U"asset/shader/grayscale";
	constexpr AssetNameView PsGradientBlur = U"asset/shader/gradient_blur";
	constexpr AssetNameView VsSoftShape = U"asset/shader/soft_shape";

	void RegisterShader();

	void RegisterAll();

	Array<Font> GetFontWithFallbacks(AssetNameView name);
}
