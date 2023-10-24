#include "stdafx.h"
#include "ConsumableItem.h"

namespace Play
{
	constexpr int consumableItemSize = static_cast<int>(ConsumableItem::Max);

	consteval std::array<ItemProps, consumableItemSize> propsData()
	{
		std::array<ItemProps, consumableItemSize> data{};

		using c = ConsumableItem;

		data[static_cast<int>(c::None)] = ItemProps{
			.item = c::None,
			.emoji = U"",
			.desc = U"(null)"
		};
		data[static_cast<int>(c::Wing)] = ItemProps{
			.item = c::Wing,
			.emoji = U"🪶",
			.desc = U"階段へワープ"
		};
		data[static_cast<int>(c::Helmet)] = ItemProps{
			.item = c::Helmet,
			.emoji = U"⛑️",
			.desc = U"ダメージを防ぐ"
		};
		data[static_cast<int>(c::Pin)] = ItemProps{
			.item = c::Pin,
			.emoji = U"📌",
			.desc = U"敵をやっつける"
		};

		return data;
	}

	const ItemProps& GetItemProps(ConsumableItem item)
	{
		constexpr static auto data = propsData();
		return data[static_cast<int>(item)];
	}
}
