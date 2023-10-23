#pragma once

#include "Forward.h"

namespace Play
{
	enum class GimmickKind
	{
		None,
		Stairs,
	};

	class GimmickGrid : public Grid<GimmickKind>
	{
	};
}
