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

	bool IsScoopAttemptInput(bool intersectsCursor);
	bool CheckScoopRequestInput(bool intersectsCursor);
	bool IsScoopCancelInput();
	Dir4Type CheckScoopMoveInput(const CharaVec2& actualPos);

	void UseItemLightBulb(ActorView self, PlayerVisionState& vision);

	void UseItemExplorer(ActorView self);

	void UpdatePlayerVision(PlayerVisionState& vision, PlayerAct act);

	void ControlPlayerBgm(const CharaVec2& pos, const MapGrid& map);

	class RocketSpark
	{
	public:
		static constexpr ColorF Yellow{U"#ffbc2e"};
		void Tick(const Vec2& center, double size);

	private:
		Array<TrailMotion> m_trails;
	};
}
