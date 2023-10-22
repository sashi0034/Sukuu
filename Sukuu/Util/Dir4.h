#pragma once

#include "Utils.h"

namespace Util
{
	namespace Dir4Index
	{
		enum : int
		{
			Right = 0,
			Up = 1,
			Left = 2,
			Down = 3,
		};
	}

	class Dir4Type : public EnumValue<int>
	{
	public:
		constexpr Dir4Type(int v): EnumValue(v) { return; }

		Vec2 ToXY() const
		{
			static constexpr std::array<Vec2, 4> a{
				Vec2{1, 0},
				Vec2{0, -1},
				Vec2{-1, 0},
				Vec2{0, 1},
			};
			return a[value()];
		}

		bool IsValid() const { return 0 <= value() && value() <= 3; }
	};

	namespace Dir4
	{
		constexpr Dir4Type Invalid{-1};
		constexpr Dir4Type Right{Dir4Index::Right};
		constexpr Dir4Type Up{Dir4Index::Up};
		constexpr Dir4Type Left{Dir4Index::Left};
		constexpr Dir4Type Down{Dir4Index::Down};

		inline Dir4Type FromIndex(int index)
		{
			return {index};
		}
	};
}
