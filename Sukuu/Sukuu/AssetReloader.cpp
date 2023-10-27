#include "stdafx.h"
#include "AssetReloader.h"

#include "AssetKeys.h"
#include "Assets.generated.h"

namespace Sukuu
{
	struct AssetReloader::Impl
	{
		DirectoryWatcher m_imageWatch{U"asset/image"};
		DirectoryWatcher m_shaderWatch{U"asset/shader"};
	};

	AssetReloader::AssetReloader() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void AssetReloader::Update()
	{
		if (p_impl->m_imageWatch.retrieveChanges())
		{
			for (auto&& path : AssetImages::GetKeys())
			{
				TextureAsset::Unregister(path);
				TextureAsset::Register(path, path);
			}
		}

		if (p_impl->m_shaderWatch.retrieveChanges())
		{
			PixelShaderAsset::UnregisterAll();
			VertexShaderAsset::UnregisterAll();
			AssetKeys::RegisterShader();
		}
	}
}
