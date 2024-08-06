#include "stdafx.h"
#include "AssetReloader.h"

#include "AssetKeys.h"
#include "Assets.generated.h"

namespace Sukuu
{
	class AssetReloaderAddon : public IAddon
	{
	public:
		bool init() override
		{
			return true;
		}

		bool update() override
		{
			if (m_imageWatch.retrieveChanges())
			{
				for (auto&& path : AssetImages::GetKeys())
				{
					TextureAsset::Unregister(path);
					TextureAsset::Register(path, path);
				}
			}

			if (m_shaderWatch.retrieveChanges())
			{
				PixelShaderAsset::UnregisterAll();
				VertexShaderAsset::UnregisterAll();
				AssetKeys::RegisterShader();
			}

			return true;
		}

	private:
		DirectoryWatcher m_imageWatch{U"asset/image"};
		DirectoryWatcher m_shaderWatch{U"asset/shader"};
	};

	void InitAssetReloader()
	{
		Addon::Register<AssetReloaderAddon>(U"AssetReloaderAddon");
	}
}
