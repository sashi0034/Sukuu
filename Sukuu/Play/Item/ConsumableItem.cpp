#include "stdafx.h"
#include "ConsumableItem.h"

namespace Play
{
	constexpr int consumableItemSize = static_cast<int>(ConsumableItem::Max);

	static consteval std::array<ItemProps, consumableItemSize> propsData()
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
			.desc = U"item_desc_wing"
		};
		data[static_cast<int>(c::Helmet)] = ItemProps{
			.item = c::Helmet,
			.emoji = U"⛑️",
			.desc = U"item_desc_helmet"
		};
		data[static_cast<int>(c::Pin)] = ItemProps{
			.item = c::Pin,
			.emoji = U"📌",
			.desc = U"item_desc_pin"
		};
		data[static_cast<int>(c::Mine)] = ItemProps{
			.item = c::Mine,
			.emoji = U"💣",
			.desc = U"item_desc_mine"
		};
		data[static_cast<int>(c::LightBulb)] = ItemProps{
			.item = c::LightBulb,
			.emoji = U"💡",
			.desc = U"item_desc_bulb"
		};
		data[static_cast<int>(c::Magnet)] = ItemProps{
			.item = c::Magnet,
			.emoji = U"🧲",
			.desc = U"item_desc_magnet"
		};
		data[static_cast<int>(c::Bookmark)] = ItemProps{
			.item = c::Bookmark,
			.emoji = U"🔖",
			.desc = U"item_desc_bookmark"
		};
		data[static_cast<int>(c::Explorer)] = ItemProps{
			.item = c::Explorer,
			.emoji = U"🗂️",
			.desc = U"item_desc_explorer"
		};
		data[static_cast<int>(c::Grave)] = ItemProps{
			.item = c::Grave,
			.emoji = U"🪦",
			.desc = U"item_desc_grave"
		};
		data[static_cast<int>(c::Sun)] = ItemProps{
			.item = c::Sun,
			.emoji = U"🌞",
			.desc = U"item_desc_sun"
		};
		data[static_cast<int>(c::Tube)] = ItemProps{
			.item = c::Tube,
			.emoji = U"🧪",
			.desc = U"item_desc_tube"
		};
		data[static_cast<int>(c::Solt)] = ItemProps{
			.item = c::Solt,
			.emoji = U"🧂",
			.desc = U"item_desc_solt"
		};
		data[static_cast<int>(c::Rocket)] = ItemProps{
			.item = c::Rocket,
			.emoji = U"🚀",
			.desc = U"item_desc_rocket"
		};

		return data;
	}

	const ItemProps& GetItemProps(ConsumableItem item)
	{
		constexpr static auto data = propsData();
		return data[static_cast<int>(item)];
	}
}
