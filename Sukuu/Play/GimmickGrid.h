#pragma once

#include "Forward.h"

namespace Play
{
	enum class GimmickKind
	{
		None,
		Stairs,
		Max,
	};

	constexpr int GimmickSize = static_cast<int>(GimmickKind::Max);

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
