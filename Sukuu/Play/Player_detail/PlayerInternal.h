#pragma once

#include "Constants.h"
#include "PlayerDefinition.h"
#include "Play/Chara/CharaUtil.h"

namespace Play::Player_detail
{
	enum class PlayerAct
	{
		Idle,
		Walk,
		Running,
		Dead,
	};

	enum class ScoopDevice
	{
		None,
		Mouse,
		Button,
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

	inline float GetCursorSize()
	{
		return Constants::CursorSize_64 / Graphics2D::GetMaxScaling();
	}

	inline RectF GetCursorRect()
	{
		return RectF(Arg::center = Cursor::PosF(), GetCursorSize());
	}

	bool IsDashingInput();

	Dir4Type CheckMoveInput();

	ScoopDevice CheckScoopEnterInput();
	bool IsScoopExitInput(ScoopDevice device);
	Dir4Type CheckScoopMoveInput(ScoopDevice device, const CharaVec2& actualPos);

	void UseItemLightBulb(ActorView self, PlayerVisionState& vision);

	void CheckUseItemExplorer(ActorView self);

	void UpdatePlayerVision(PlayerVisionState& vision, PlayerAct act);

	void ControlPlayerBgm(const CharaVec2& pos, const MapGrid& map);
}
