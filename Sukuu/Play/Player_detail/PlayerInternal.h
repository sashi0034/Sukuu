#pragma once

namespace Play
{
	struct PlayerImmortality
	{
		double immortalTime{};
		int immortalStock{};

		void Reset()
		{
			immortalTime = 0;
			immortalStock = 0;
		}

		bool IsImmortal() const
		{
			return immortalTime > 0 || immortalStock > 0;
		}
	};
}
