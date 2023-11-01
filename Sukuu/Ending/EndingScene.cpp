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
	double m_cameraX{};

	void Init(ActorView self)
	{
		m_bg = self.AsParent().Birth(EndingBackground());
		m_bg.Init();
	}

	void Update(ActorBase& self)
	{
		m_cameraX += Scene::DeltaTime() * 4.0;

		const int mapCenterY = (m_bg.PlainSize().y / 2) - Px_16 / 2;
		const auto cameraPos = Vec2{m_cameraX, mapCenterY};
		const double cameraScale = 4.5 + 0.5 * Periodic::Sine1_1(12.0s);
		const auto center = Vec2{0, Scene::Center().y};
		const Transformer2D transformer0{
			Mat3x2::Translate(center).translated(-cameraPos).scaled(cameraScale, center)
		};

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
