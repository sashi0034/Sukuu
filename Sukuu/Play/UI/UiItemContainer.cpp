#include "stdafx.h"
#include "UiItemContainer.h"

#include "UiItemButton.h"
#include "Gm/GamepadObserver.h"
#include "Play/PlayCore.h"
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
	int m_gamepadIndexing{};

	void Update(ActorView self)
	{
		const auto center = Point{Scene::Center().x, getToml<int>(U"base_vertical")};
		const Transformer2D transform{Mat3x2::Translate(m_offset).scaled(m_scale, center)};
		auto&& player = PlayCore::Instance().GetPlayer();
		const auto& playerItems = player.PersonalData().items;

		if (Gm::IsUsingGamepad())
		{
			// ゲームパッドで動かす
			if (IsGamepadDown(Gm::GamepadButton::LB)) m_gamepadIndexing--;
			if (IsGamepadDown(Gm::GamepadButton::RB)) m_gamepadIndexing++;
			if (m_gamepadIndexing < 0) m_gamepadIndexing = MaxItemPossession - 1;
			if (m_gamepadIndexing >= MaxItemPossession) m_gamepadIndexing = 0;
		}

		for (int i = 0; i < m_items.size(); ++i)
		{
			// 上画面中央に隙間が開くようにアイテム位置を決定する
			const int itemSizeOneMore = m_items.size() + 1;
			const int itemIndexPadding = i >= itemSizeOneMore / 2 ? 1 : 0;
			const auto centerOffset = Vec2{(i + itemIndexPadding - itemSizeOneMore / 2.0) * 96, 0}.asPoint();

			// アイテム更新
			m_items[i].Tick({
				.label = m_itemLabel,
				.index = i,
				.gamepadIndexing = Gm::IsUsingGamepad() ? m_gamepadIndexing : -1,
				.center = center.movedBy(centerOffset),
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

	void UiItemContainer::Init(int itemIndexing)
	{
		p_impl->m_gamepadIndexing = itemIndexing;
	}

	void UiItemContainer::Update()
	{
		ActorBase::Update();
		p_impl->Update(*this);
	}

	int UiItemContainer::GetIndexing() const
	{
		return p_impl->m_gamepadIndexing;
	}
}
