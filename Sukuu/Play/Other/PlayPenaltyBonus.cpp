#include "stdafx.h"
#include "PlayPenaltyBonus.h"

namespace Play
{
	double GetEnemyKilledBonusHeal(int killedIndex)
	{
		static constexpr std::array bonus = {15, 20, 30, 45};
		return bonus[std::min(killedIndex, static_cast<int>(bonus.size()) - 1)];
	}

	double GetPlayerScoopedPenaltyDamage(int count)
	{
		static constexpr std::array penalty = {5, 10, 15, 20, 25};
		return penalty[std::min(count, static_cast<int>(penalty.size()) - 1)];
	}
}
