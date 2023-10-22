#pragma once

namespace AssetImages
{
	constexpr StringView brick_stylish_24x24 = U"asset/image/brick_stylish_24x24.png";
	constexpr StringView magma_tile_24x24 = U"asset/image/magma_tile_24x24.png";
	constexpr StringView phine_32x32 = U"asset/image/phine_32x32.png";
	constexpr StringView punicat_24x24 = U"asset/image/punicat_24x24.png";

	inline void RegisterAll()
	{
		TextureAsset::Register(brick_stylish_24x24, brick_stylish_24x24);
		TextureAsset::Register(magma_tile_24x24, magma_tile_24x24);
		TextureAsset::Register(phine_32x32, phine_32x32);
		TextureAsset::Register(punicat_24x24, punicat_24x24);
	}
}
