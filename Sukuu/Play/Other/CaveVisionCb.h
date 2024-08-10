#pragma once

#include "Assets.generated.h"
#include "AssetKeys.h"

namespace Play
{
	// cave_vision は 96-px ごとに周期発生

	struct CaveVisionCb
	{
		float animRate = 0;
		Float2 positionOffset{0, 12}; // 12 = (1080 % 96) / 2
	};

	[[nodiscard]] inline ScopedCustomShader2D PrepareCaveVision(const ConstantBuffer<CaveVisionCb>& cb)
	{
		Graphics2D::SetPSConstantBuffer(1, cb);

		Graphics2D::SetPSTexture(1, TextureAsset(AssetImages::aqua_noise));
		Graphics2D::SetPSTexture(2, TextureAsset(AssetImages::cosmos_noise));

		return ScopedCustomShader2D{PixelShaderAsset(AssetKeys::PsCaveVision)};
	}
}
