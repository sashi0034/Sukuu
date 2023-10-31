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
	double m_playerX = -Px_16 * 2;
	Play::AnimTimer m_animTImer{};

	void Init(ActorView self)
	{
		m_bg = self.AsParent().Birth(EndingBackground());
		m_bg.Init();
	}

	void Update(ActorBase& self)
	{
		m_animTImer.Tick();

		m_cameraX += Scene::DeltaTime() * 4.0;
		m_playerX += Scene::DeltaTime() * 6.0;

		const int mapCenterY = (m_bg.PlainSize().y / 2) - Px_16 / 2;
		const auto cameraPos = Vec2{m_cameraX, mapCenterY};
		const Transformer2D transformer0{Mat3x2::Scale(5, cameraPos).translated(cameraPos)};

		self.ActorBase::Update();

		[&]
		{
			const ScopedRenderStates2D state{SamplerState::BorderNearest};
			Play::GetUsualPlayerTexture(Dir4::Right, m_animTImer, true).draw(Vec2{m_playerX, mapCenterY});
		}();
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
