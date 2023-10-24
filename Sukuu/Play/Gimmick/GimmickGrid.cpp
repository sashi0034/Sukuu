#include "stdafx.h"

#include "GimmickGrid.h"

struct Play::GimmickGrid::Impl
{
	std::array<Point, GimmickSize> m_singlePoints{};

	Point FindSingle(const GimmickGrid& self, GimmickKind target)
	{
		auto&& cache = m_singlePoints[static_cast<int>(target)];
		if (self[cache] == target) return cache;
		for (const auto p : step(self.size()))
		{
			if (self[p] == target)
			{
				cache = p;
				break;
			}
		}
		return cache;
	}
};

namespace Play
{
	GimmickGrid::GimmickGrid() :
		p_impl(std::make_shared<Impl>())
	{
	}

	Point GimmickGrid::GetSinglePoint(GimmickKind target) const
	{
		return p_impl->FindSingle(*this, target);
	}
}
