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
		Item_Mine,
		Item_LightBulb,
		Item_Magnet,
		Item_Bookmark,
		Item_Explorer,
		Item_Grave,
		Item_Sun,
		Item_Tube,
		Item_Solt,
		Installed_Mine,
		Installed_Magnet,

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
		case GimmickKind::Item_Mine:
			return ConsumableItem::Mine;
		case GimmickKind::Item_LightBulb:
			return ConsumableItem::LightBulb;
		case GimmickKind::Item_Magnet:
			return ConsumableItem::Magnet;
		case GimmickKind::Item_Bookmark:
			return ConsumableItem::Bookmark;
		case GimmickKind::Item_Explorer:
			return ConsumableItem::Explorer;
		case GimmickKind::Item_Grave:
			return ConsumableItem::Grave;
		case GimmickKind::Item_Sun:
			return ConsumableItem::Sun;
		case GimmickKind::Item_Tube:
			return ConsumableItem::Tube;
		case GimmickKind::Item_Solt:
			return ConsumableItem::Solt;
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
