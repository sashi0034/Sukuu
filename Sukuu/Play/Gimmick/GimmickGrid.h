﻿#pragma once

#include "Play/Item/ConsumableItem.h"
#include "Play/Forward.h"
#include "Util/Dir4.h"

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
		Item_Rocket,
		SemiItem_Hourglass,
		SemiItem_Vessel,
		SemiItem_Book,
		Installed_Mine,
		Installed_Magnet,
		Installed_Grave,
		Arrow_right,
		Arrow_up,
		Arrow_left,
		Arrow_down,
		DemiArrow_right,
		DemiArrow_up,
		DemiArrow_left,
		DemiArrow_down,
		Tree_small,
		Tree_large,

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
		case GimmickKind::Item_Rocket:
			return ConsumableItem::Rocket;
		default: ;
			return ConsumableItem::None;
		}
	}

	inline bool IsSemiItemGimmick(GimmickKind gimmick)
	{
		return
			gimmick == GimmickKind::SemiItem_Hourglass ||
			gimmick == GimmickKind::SemiItem_Vessel ||
			gimmick == GimmickKind::SemiItem_Book;
	}

	inline Dir4Type GimmickArrowToDir(GimmickKind gimmick)
	{
		switch (gimmick)
		{
		case GimmickKind::Arrow_right:
			return Dir4::Right;
		case GimmickKind::Arrow_up:
			return Dir4::Up;
		case GimmickKind::Arrow_left:
			return Dir4::Left;
		case GimmickKind::Arrow_down:
			return Dir4::Down;
		case GimmickKind::DemiArrow_right:
			return Dir4::Right;
		case GimmickKind::DemiArrow_up:
			return Dir4::Up;
		case GimmickKind::DemiArrow_left:
			return Dir4::Left;
		case GimmickKind::DemiArrow_down:
			return Dir4::Down;
		default:
			return Dir4::Invalid;
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
