#include "stdafx.h"
#include "TitleLogo.h"

#include "AssetKeys.h"

namespace
{
}

struct Title::TitleLogo::Impl
{
	void Update()
	{
		TextureAsset(AssetKeys::title_logo).scaled(3.0).drawAt(Vec2{Scene::Center().x, Scene::Center().y / 2});
	}
};

namespace Title
{
	TitleLogo::TitleLogo() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void TitleLogo::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	double TitleLogo::OrderPriority() const
	{
		return 1000.0;
	}
}
