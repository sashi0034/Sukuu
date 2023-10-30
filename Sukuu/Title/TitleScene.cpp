#include "stdafx.h"
#include "TitleScene.h"

#include "AssetKeys.h"
#include "GitRevision.h"
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
		FontAsset(AssetKeys::RocknRoll_24_Bitmap)(U"Version git-" + GitRevisionLiteral)
			.draw(Arg::bottomLeft = Scene::Size().y0().yx());

		if (MouseL.down()) m_concluded = true;
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

	bool TitleScene::IsConcluded() const
	{
		return p_impl->m_concluded;
	}
}
