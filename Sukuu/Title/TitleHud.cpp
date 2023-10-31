#include "stdafx.h"
#include "TitleHud.h"

#include "AssetKeys.h"

namespace
{
}

struct Title::TitleHud::Impl
{
	void Update()
	{
		TextureAsset(AssetKeys::title_logo).scaled(3.0).drawAt(Vec2{Scene::Center().x, Scene::Center().y / 2});
	}
};

namespace Title
{
	TitleHud::TitleHud() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void TitleHud::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	double TitleHud::OrderPriority() const
	{
		return 1000.0;
	}
}
