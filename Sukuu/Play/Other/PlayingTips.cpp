#include "stdafx.h"
#include "PlayingTips.h"

#include "Gm/GamepadObserver.h"

namespace
{
	const Array<StringView> basicTips{
		U"tips_item_helmet",
		U"tips_item_pin_1",
		U"tips_item_pin_2",
		U"tips_item_magnet",
		U"tips_use_sukuu",
		U"tips_through_wall_with_sukuu",
		U"tips_sleeping_enemy",
		U"tips_effective_sukuu",
		U"tips_one_blow",
		U"tips_clear_floor",
		U"tips_heart_appears",
		U"tips_about_maze",
		U"tips_use_item_while_moving",
	};

	const Array<StringView> advancedTips{
		U"tips_item_grave_1",
		U"tips_item_grave_2",
		U"tips_item_sun",
		U"tips_camera_movement_settings",
		U"tips_running_sideeffect"
	};

	const Array<StringView> keyboardAndMouseTips{
		U"tips_turn_by_mouse",
	};
}

namespace Play
{
	StringView GetPlayingTips(int floorIndex)
	{
		Array<StringView> candidates{};
		candidates.append(basicTips);

		if (floorIndex >= 15)
		{
			candidates.append(advancedTips);

			if (Gm::IsUsingGamepad()) candidates.append(keyboardAndMouseTips);
		}

		return candidates[Random(0, static_cast<int>(candidates.size()) - 1)];
	}

	Array<StringView> GetAllPlayingTips()
	{
		Array<StringView> tips{};
		tips.append(basicTips);
		tips.append(advancedTips);
		tips.append(keyboardAndMouseTips);
		return tips;
	}
}
