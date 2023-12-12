#include "stdafx.h"
#include "GamepadRegistering.h"

#include "AssetKeys.h"
#include "Constants.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"gm.gamepad_registering." + key);
	}
}

void loopInternal()
{
	while (System::Update())
	{
		Util::RefreshTomlParameters();
		Scene::SetBackground(ColorF(Constants::SoftDarkblue) * getToml<double>(U"bg_brightness"));

		ClearPrint();
		Print(Cursor::Pos());

		TextureAsset(AssetKeys::gamepad).resized(1024).drawAt(Scene::Center());
	}
}

void Gm::DialogGamepadRegistering()
{
	const auto beforeBg = Scene::GetBackground();
	loopInternal();
	Scene::SetBackground(beforeBg);
}
