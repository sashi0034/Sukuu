#include "stdafx.h"
#include "PlayScene.h"

#include "Params.h"

namespace Play
{
	struct PlayScene::Impl
	{
		Texture t1{};
	};

	PlayScene::PlayScene() :
		p_impl(std::make_shared<Impl>())
	{
		p_impl->t1 = static_cast<Texture>(TextureAsset(AssetImages::brick_stylish_24x24));
	}

	void PlayScene::Update()
	{
		ActorBase::Update();
		p_impl->t1.scaled(4).drawAt(Scene::Center());
	}
}
