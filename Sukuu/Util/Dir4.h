#pragma once

#include "Utilities.h"

namespace Util
{
	class Dir4Type : public EnumValue<int>
	{
	public:
		enum Index : int
		{
			Right = 0,
			Up = 1,
			Left = 2,
			Down = 3,
		};

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

		Dir4Type RotatedL() const { return Dir4Type((value() + 3) % 4); }
		Dir4Type RotatedR() const { return Dir4Type((value() + 1) % 4); }
		Dir4Type Rotated(bool isLeft) const { return isLeft ? RotatedL() : RotatedR(); }
		Dir4Type Reversed() const { return Dir4Type((value() + 2) % 4); }

		Index GetIndex() const { return static_cast<Index>(value()); }
		bool IsValid() const { return 0 <= value() && value() <= 3; }
	};

	namespace Dir4
	{
		constexpr Dir4Type Invalid{-1};
		constexpr Dir4Type Right{Dir4Type::Right};
		constexpr Dir4Type Up{Dir4Type::Up};
		constexpr Dir4Type Left{Dir4Type::Left};
		constexpr Dir4Type Down{Dir4Type::Down};

		inline Dir4Type FromIndex(int index)
		{
			return {index};
		}
	};
}
