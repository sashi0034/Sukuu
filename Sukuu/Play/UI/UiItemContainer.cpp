#include "stdafx.h"
#include "UiItemContainer.h"

#include "UiItemButton.h"
#include "Play/PlayScene.h"
#include "Util/CoroUtil.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.ui_item_container." + key);
	}
}

struct Play::UiItemContainer::Impl
{
	std::array<UiItemButton, MaxItemPossession> m_items;
	Vec2 m_offset{};
	double m_scale = 1.0;
	CoroActor m_animation{};
	UiItemLabel m_itemLabel{};

	void Update(ActorView self)
	{
		const auto center = Point{Scene::Center().x, getToml<int>(U"base_vertical")};
		const Transformer2D transform{Mat3x2::Translate(m_offset).scaled(m_scale, center)};
		auto&& player = PlayScene::Instance().GetPlayer();
		const auto& playerItems = player.PersonalData().items;

		for (int i = 0; i < m_items.size(); ++i)
		{
			m_items[i].Tick({
				.label = m_itemLabel,
				.index = (i + 1),
				.center = center.movedBy({(i - m_items.size() / 2) * 96, 0}),
				.item = playerItems[i],
				.canUse = [&]() { return player.CanUseItem(i); },
				.requestUse = [&]() { player.RequestUseItem(i); }
			});
		}

		m_itemLabel.SetCenter(center + getToml<Vec2>(U"label_center"));
		m_itemLabel.Update();
	}

private:
	void animUnused(YieldExtended yield, ActorView self)
	{
		StartCoro(self, [&](YieldExtended yield1)
		{
			yield1.WaitForExpire(
				AnimateEasing<EaseOutBack>(self, &m_scale, 2.0, 0.6));
			yield1.WaitForExpire(
				AnimateEasing<EaseInBack>(self, &m_scale, 1.0, 0.6));
		});

		yield.WaitForExpire(
			AnimateEasing<EaseOutBack>(self, &m_offset, {0, 80}, 0.6));
		yield.WaitForExpire(
			AnimateEasing<EaseOutBack>(self, &m_offset, {0, 0}, 0.6));
	}
};

namespace Play
{
	UiItemContainer::UiItemContainer() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void UiItemContainer::Update()
	{
		ActorBase::Update();
		p_impl->Update(*this);
	}
}
