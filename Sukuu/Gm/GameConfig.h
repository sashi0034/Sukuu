#pragma once
#include "GamepadButton.h"

namespace Gm
{
	struct GameConfig
	{
		struct
		{
			HashTable<String, GamepadButtonMapping> mapping;
		} gamepad;

		void RequestWrite();
		static GameConfig& Instance();
	};
}
