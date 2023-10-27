#pragma once

#include "Play/Item/ConsumableItem.h"
#include "Play/Forward.h"

namespace Play
{
	enum class GimmickKind
	{
		None,
		Stairs,
		Item_Wing,
		Item_Helmet,
		Item_Pin,

		Max,
	};

	constexpr int GimmickSize = static_cast<int>(GimmickKind::Max);

	inline ConsumableItem GimmickToItem(GimmickKind gimmick)
	{
		switch (gimmick)
		{
		case GimmickKind::Item_Wing:
			return ConsumableItem::Wing;
		case GimmickKind::Item_Helmet:
			return ConsumableItem::Helmet;
		case GimmickKind::Item_Pin:
			return ConsumableItem::Pin;
		default: ;
			return ConsumableItem::None;
		}
	}

	class GimmickGrid : public Grid<GimmickKind>
	{
	public:
		GimmickGrid();
		Point GetSinglePoint(GimmickKind target) const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
