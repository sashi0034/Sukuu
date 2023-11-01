#include "stdafx.h"
#include "EndingScene.h"

#include "Assets.generated.h"
#include "EndingBackground.h"
#include "Play/Chara/CharaUtil.h"
#include "Play/Player_detail/PlayerAnimation.h"
#include "Util/ActorContainer.h"

namespace
{
}

struct Ending::EndingScene::Impl
{
	bool m_finished{};
	EndingBackground m_bg{};


	void Init(ActorView self)
	{
		m_bg = self.AsParent().Birth(EndingBackground());
		m_bg.Init();
	}

	void Update(ActorBase& self)
	{
		self.ActorBase::Update();
	}
};

namespace Ending
{
	EndingScene::EndingScene() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void EndingScene::Init()
	{
		p_impl->Init(*this);
	}

	void EndingScene::Update()
	{
		p_impl->Update(*this);
	}

	bool EndingScene::IsFinished() const
	{
		return p_impl->m_finished;
	}
}
