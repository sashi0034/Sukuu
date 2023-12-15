#pragma once
#include "GamepadButton.h"
#include "Util/Utilities.h"

namespace Gm
{
	enum class GameLanguage
	{
		Ja = 0,
		En = 1,
		Cs = 2,
		Ct = 3,
		Ko = 4,
		Max,
	};

	class TenStepNumber : public Util::EnumValue<int>
	{
	public:
		TenStepNumber(int v) : EnumValue(std::max(std::min(v, 10), 0)) { return; }

		double GetRate(double base = 5.0) const
		{
			return value() / base;
		}
	};

	struct GameConfig
	{
		bool fullscreen{};
		GameLanguage language{};

		TenStepNumber bgm_volume{5};
		TenStepNumber se_volume{5};
		TenStepNumber camera_move{2};

		HashTable<String, GamepadButtonMapping> gamepad_mapping{};

		void RequestWrite();
		static GameConfig& Instance();
	};
}
