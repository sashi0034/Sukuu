#include "stdafx.h"
#include "GameCursor.h"

#include "Assets.generated.h"
#include "Constants.h"
#include "GamepadButton.h"
#include "GamepadObserver.h"
#include "Util/Utilities.h"

namespace
{
	bool s_hide{};
	Vec2 s_gamepadCursor{};
}

namespace Gm
{
	void RequestHideGameCursor()
	{
		s_hide = true;
	}

	void UpdateGameCursor()
	{
		Cursor::RequestStyle(CursorStyle::Hidden);
		if (not s_hide)
		{
			TextureAsset(AssetImages::cursor).resized(Point::One() * Constants::CursorSize_64).drawAt(Cursor::PosF());
		}
		s_hide = false;
	}

	// void MoveCursorByGamepad()
	// {
	// 	const auto gamepad = Gamepad(GamepadPlayer_0);
	// 	if (not gamepad) return;
	//
	// 	if (s_gamepadCursor.asPoint() != Cursor::Pos())
	// 	{
	// 		s_gamepadCursor = Cursor::Pos();
	// 	}
	//
	// 	s_gamepadCursor = s_gamepadCursor + 2048 * GetGamepadAxeL() * Scene::DeltaTime();
	// 	s_gamepadCursor.clamp(Rect(Scene::Size()).stretched(-Constants::CursorSize_64 / 2));
	// 	Cursor::SetPos(s_gamepadCursor.asPoint());
	// 	// FIXME: https://github.com/Siv3D/OpenSiv3D/issues/1167
	// }

	bool CheckConfirmSimply()
	{
		return IsUsingGamepad()
			       ? IsGamepadDown(GamepadButton::A)
			       : Util::IsSceneLeftClicked();
	}
}
