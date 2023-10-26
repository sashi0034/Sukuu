#include "stdafx.h"
#include "TitleScene.h"

#include "TitleBackground.h"
#include "Util/ActorContainer.h"


namespace
{
	template <typename T>
	T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"title.scene." + key);
	}
}

struct Title::TitleScene::Impl
{
	bool m_concluded{};
	TitleBackground m_bg{};

	void Init(ActorBase& self)
	{
		m_bg = self.AsParent().Birth(TitleBackground());
		m_bg.Init();
	}

	void Update()
	{
	}
};

namespace Title
{
	TitleScene::TitleScene() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void TitleScene::Init()
	{
		p_impl->Init(*this);
	}

	void TitleScene::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	bool TitleScene::IsConcluded()
	{
		return p_impl->m_concluded;
	}
}
