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
		data[static_cast<int>(c::Mine)] = ItemProps{
			.item = c::Mine,
			.emoji = U"💣",
			.desc = U"地雷を置く"
		};
		data[static_cast<int>(c::LightBulb)] = ItemProps{
			.item = c::LightBulb,
			.emoji = U"💡",
			.desc = U"霧払いをする"
		};
		data[static_cast<int>(c::Magnet)] = ItemProps{
			.item = c::Magnet,
			.emoji = U"🧲",
			.desc = U"敵を足止める"
		};
		data[static_cast<int>(c::Bookmark)] = ItemProps{
			.item = c::Bookmark,
			.emoji = U"🔖",
			.desc = U"マップに階段とアイテム位置表示"
		};
		data[static_cast<int>(c::Explorer)] = ItemProps{
			.item = c::Explorer,
			.emoji = U"🗂️",
			.desc = U"マップに敵の位置を表示"
		};
		data[static_cast<int>(c::Grave)] = ItemProps{
			.item = c::Grave,
			.emoji = U"🪦",
			.desc = U"敵の進路をふさぐ"
		};
		data[static_cast<int>(c::Sun)] = ItemProps{
			.item = c::Sun,
			.emoji = U"🌞",
			.desc = U"炎で敵を焼き尽くす"
		};
		data[static_cast<int>(c::Tube)] = ItemProps{
			.item = c::Tube,
			.emoji = U"🧪",
			.desc = U"体力を回復する"
		};
		data[static_cast<int>(c::Solt)] = ItemProps{
			.item = c::Solt,
			.emoji = U"🧂",
			.desc = U"敵に見つかりにくくなる"
		};

		return data;
	}

	const ItemProps& GetItemProps(ConsumableItem item)
	{
		constexpr static auto data = propsData();
		return data[static_cast<int>(item)];
	}
}
