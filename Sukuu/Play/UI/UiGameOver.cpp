#include "stdafx.h"
#include "UiGameOver.h"

#include "Constants.h"
#include "Play/PlayScene.h"
#include "Util/CoroUtil.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	using namespace Play;

	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.ui_game_over." + key);
	}
}

struct UiGameOver::Impl
{
	int m_floorIndex{};
	double m_bgAlpha{};

	void Update()
	{
		if (m_bgAlpha == 0) return;
		(void)Rect(Scene::Size()).draw(ColorF(Constants::HardDarkblue, m_bgAlpha));
	}

	ActorWeak StartPerform(ActorView self)
	{
		return StartCoro(self, [this, self](YieldExtended yield)
		{
			perform(yield, self);
		});
	}

private:
	void perform(YieldExtended& yield, ActorView self)
	{
		yield.WaitForDead(
			AnimateEasing<EaseInSine>(self, &m_bgAlpha, 1.0, 1.5));
		PlayScene::Instance().GetEnemies().KillAll();
		yield.WaitForTime(2.0);
		// TODO
	}
};

namespace Play
{
	UiGameOver::UiGameOver() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void UiGameOver::Init(int floorIndex)
	{
		p_impl->m_floorIndex = floorIndex;
	}

	void UiGameOver::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	double UiGameOver::OrderPriority() const
	{
		return 100000.0;
	}

	ActorWeak UiGameOver::StartPerform()
	{
		return p_impl->StartPerform(*this);
	}
}
