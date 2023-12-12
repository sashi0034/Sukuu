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

	struct GamepadRegisteringState
	{
	};

	void drawTexts()
	{
		FontAsset(AssetKeys::RocknRoll_Sdf)(U"コントローラー設定")
			.drawAt(getToml<int>(U"font_size"), Vec2{Scene::Center().x, getToml<int>(U"top_y")}, Palette::Gray);
	}

	void updateGamepad()
	{
		Transformer2D transformer2D{
			Mat3x2::Scale(getToml<double>(U"gamepad_scale"), Scene::Center().movedBy(0, getToml<int>(U"gamepad_y")))
		};
		TextureAsset(AssetKeys::gamepad).resized(1024).drawAt(Scene::Center());
	}

	void loopInternal()
	{
		while (System::Update())
		{
			Util::RefreshTomlParameters();
			Scene::SetBackground(getToml<ColorF>(U"bg"));

			ClearPrint();
			Print(Cursor::Pos());

			drawTexts();

			updateGamepad();
		}
	}
}

void Gm::DialogGamepadRegistering()
{
	const auto beforeBg = Scene::GetBackground();
	loopInternal();
	Scene::SetBackground(beforeBg);
}
