#pragma once
#include "GamepadButton.h"
#include "Util/Utilities.h"

namespace Gm
{
	enum class GameLanguage
	{
		Ja,
		En,
		Cs,
		Ct,
		Ko,
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

		struct
		{
			TenStepNumber bgm{5};
			TenStepNumber se{5};
		} volume{};

		GameLanguage language;

		struct
		{
			HashTable<String, GamepadButtonMapping> mapping;
		} gamepad{};

		struct
		{
			TenStepNumber amount{5};
		} camera;

		void RequestWrite();
		static GameConfig& Instance();
	};
}
