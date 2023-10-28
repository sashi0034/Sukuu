#pragma once

#include "PlayerDefinition.h"

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

	bool CheckUseItemLightBulb(ActorView self, PlayerVisionState& vision);

	bool CheckUseItemExplorer(ActorView self);
}
