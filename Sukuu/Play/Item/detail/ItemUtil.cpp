#include "stdafx.h"
#include "ItemUtil.h"

namespace Play::Item_detail
{
	double GetItemCharaOrderPriority(const CharaPosition& pos, const Size& size)
	{
		return 1000.0 + ((pos.viewPos.y - (CellPx_24 - size.y) / 2) / (CellPx_24 * 1024));
	}

	RectF GetItemCollider(const CharaPosition& pos, const Size& size)
	{
		return RectF{pos.actualPos.movedBy(Size{CellPx_24, CellPx_24} - size) / 2, size};
	}
}
