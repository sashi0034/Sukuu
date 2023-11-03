#include "stdafx.h"
#include "EndingScene.h"

#include "EndingBackground.h"
#include "EndingHud.h"
#include "Play/Player_detail/PlayerAnimation.h"
#include "Util/ActorContainer.h"

struct Ending::EndingScene::Impl
{
	EndingBackground m_bg{};
	EndingHud m_hud{};

	void Init(ActorView self, const Play::MeasuredSecondsArray& measured)
	{
		m_bg = self.AsParent().Birth(EndingBackground());
		m_bg.Init();

		m_hud = self.AsParent().Birth(EndingHud());
		m_hud.Init(measured);
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

	void EndingScene::Init(const Play::MeasuredSecondsArray& measured)
	{
		p_impl->Init(*this, measured);
	}

	void EndingScene::Update()
	{
		p_impl->Update(*this);
	}

	bool EndingScene::IsFinished() const
	{
		return p_impl->m_hud.IsFinished();
	}
}
