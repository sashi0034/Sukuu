#pragma once

#include "Assets.generated.h"
#include "AssetKeys.h"

namespace Play
{
	struct CaveVisionCb
	{
		float animRate = 0;
		bool masked{true};
		Float2 positionOffset{};
	};

	[[nodiscard]] inline ScopedCustomShader2D PrepareCaveVision(const ConstantBuffer<CaveVisionCb>& cb)
	{
		Graphics2D::SetPSConstantBuffer(1, cb);

		Graphics2D::SetPSTexture(1, TextureAsset(AssetImages::aqua_noise));
		Graphics2D::SetPSTexture(2, TextureAsset(AssetImages::cosmos_noise));

		return ScopedCustomShader2D{PixelShaderAsset(AssetKeys::PsCaveVision)};
	}
}
