#pragma once

namespace AssetImages
{
	constexpr StringView beatrice_32x32 = U"asset/image/beatrice_32x32.png";
	constexpr StringView bookmark_16x16 = U"asset/image/bookmark_16x16.png";
	constexpr StringView brick_stylish_24x24 = U"asset/image/brick_stylish_24x24.png";
	constexpr StringView brick_stylish_light_24x24 = U"asset/image/brick_stylish_light_24x24 .png";
	constexpr StringView clock_16x16 = U"asset/image/clock_16x16.png";
	constexpr StringView helmet_16x16 = U"asset/image/helmet_16x16.png";
	constexpr StringView hourglass_16x16 = U"asset/image/hourglass_16x16.png";
	constexpr StringView light_bulb_16x16 = U"asset/image/light_bulb_16x16.png";
	constexpr StringView magma_tile_24x24 = U"asset/image/magma_tile_24x24.png";
	constexpr StringView magnet_16x16 = U"asset/image/magnet_16x16.png";
	constexpr StringView phine_32x32 = U"asset/image/phine_32x32.png";
	constexpr StringView pin_16x16 = U"asset/image/pin_16x16.png";
	constexpr StringView punicat_24x24 = U"asset/image/punicat_24x24.png";
	constexpr StringView solt_16x16 = U"asset/image/solt_16x16.png";
	constexpr StringView stairs_24x24 = U"asset/image/stairs_24x24.png";
	constexpr StringView test_tube_16x16 = U"asset/image/test_tube_16x16.png";
	constexpr StringView tex_aqua_noise = U"asset/image/tex_aqua_noise.png";
	constexpr StringView tex_cosmos_noise = U"asset/image/tex_cosmos_noise.png";
	constexpr StringView wing_16x16 = U"asset/image/wing_16x16.png";

	inline Array<StringView> GetKeys()
	{
		return {
			beatrice_32x32,
			bookmark_16x16,
			brick_stylish_24x24,
			brick_stylish_light_24x24,
			clock_16x16,
			helmet_16x16,
			hourglass_16x16,
			light_bulb_16x16,
			magma_tile_24x24,
			magnet_16x16,
			phine_32x32,
			pin_16x16,
			punicat_24x24,
			solt_16x16,
			stairs_24x24,
			test_tube_16x16,
			tex_aqua_noise,
			tex_cosmos_noise,
			wing_16x16,
		};
	}
}
