#include "stdafx.h"
#include "UiItemContainer.h"

#include "AssetKeys.h"
#include "UiItemButton.h"
#include "Util/CoroUtil.h"
#include "Util/EasingAnimation.h"
#include "Util/TomlParametersWrapper.h"

struct Play::UiItemContainer::Impl
{
	std::array<UiItemButton, 9> m_items;
	Vec2 m_offset{};
	double m_scale = 1.0;
	CoroActor m_animation{};
	UiItemLabel m_itemLabel{};

	void Update(ActorBase& self)
	{
		const auto center = Point{Scene::Center().x, 60};
		const Transformer2D transform{Mat3x2::Translate(m_offset).scaled(m_scale, center)};

		for (int i = 0; i < m_items.size(); ++i)
		{
			m_items[i].Tick({
				.label = m_itemLabel,
				.index = (i + 1),
				.font = static_cast<Font>(FontAsset(AssetKeys::F24)),
				.center = center.movedBy({(i - m_items.size() / 2) * 80, 0}),
				.icon = static_cast<Texture>(TextureAsset(U"😎"))
			});
		}

		m_itemLabel.SetCenter(center + GetTomlParameter<Point>(U"play.ui_item_container.center"));
		m_itemLabel.Update();

		if (KeyZ.down() && m_animation.IsDead())
		{
			m_animation = StartCoro(self, [&](CoroTaskYield& yield)
			{
				startAnim(yield, self);
			});
		}
	}

private:
	void startAnim(YieldExtended yield, ActorBase& self)
	{
		StartCoro(self, [&](YieldExtended yield1)
		{
			yield1.WaitForDead(
				AnimateEasing<EaseOutBack>(self, &m_scale, 2.0, 0.6));
			yield1.WaitForDead(
				AnimateEasing<EaseInBack>(self, &m_scale, 1.0, 0.6));
		});

		yield.WaitForDead(
			AnimateEasing<EaseOutBack>(self, &m_offset, {0, 80}, 0.6));
		yield.WaitForDead(
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
