#pragma once

namespace AssetImages
{
	constexpr StringView beatrice_32x32 = U"asset/image/beatrice_32x32.png";
	constexpr StringView brick_stylish_24x24 = U"asset/image/brick_stylish_24x24.png";
	constexpr StringView magma_tile_24x24 = U"asset/image/magma_tile_24x24.png";
	constexpr StringView phine_32x32 = U"asset/image/phine_32x32.png";
	constexpr StringView punicat_24x24 = U"asset/image/punicat_24x24.png";
	constexpr StringView stairs_24x24 = U"asset/image/stairs_24x24.png";

	inline void RegisterAll()
	{
		TextureAsset::Register(beatrice_32x32, beatrice_32x32);
		TextureAsset::Register(brick_stylish_24x24, brick_stylish_24x24);
		TextureAsset::Register(magma_tile_24x24, magma_tile_24x24);
		TextureAsset::Register(phine_32x32, phine_32x32);
		TextureAsset::Register(punicat_24x24, punicat_24x24);
		TextureAsset::Register(stairs_24x24, stairs_24x24);
	}
}
