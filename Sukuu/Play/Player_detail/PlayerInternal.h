#pragma once

#include "PlayerDefinition.h"
#include "Play/Chara/CharaUtil.h"

namespace Play
{
	enum class PlayerAct
	{
		Idle,
		Walk,
		Running,
		Dead,
	};

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

	void updateVision(PlayerVisionState& vision, PlayerAct act);
}
