#pragma once

namespace Play
{
	enum class ConsumableItem
	{
		None,
		Wing,
		Helmet,
		Pin,

		Max,
	};

	struct ItemProps
	{
		ConsumableItem item;
		AssetNameView emoji;
		StringView desc;
	};

	const ItemProps& GetItemProps(ConsumableItem item);
}
