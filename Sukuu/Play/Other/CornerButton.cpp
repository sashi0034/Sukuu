#include "stdafx.h"
#include "CornerButton.h"

#include "AssetKeys.h"
#include "Constants.h"
#include "Gm/GamepadObserver.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.corner_button." + key);
	}

	void drawButton(const Rect& rect, bool hover)
	{
		rect
			.drawShadow({6, 6}, 24, 3)
			.rounded(20).draw(getToml<ColorF>(U"button_color") * (hover ? 1.3 : 1.0));
	};
}

namespace Play
{
	void DrawButtonFrame(const RectF& region)
	{
		region.stretched(8).rounded(8).drawFrame(4 + 4 * Periodic::Sine0_1(1.0s), Palette::Gold);
	}

	CornerButton::Hovered CornerButton::Update(int index, int cursorIndex)
	{
		// ボタン描画基本処理
		const auto buttonSize = getToml<Size>(U"button_size");
		const int buttonSpace = getToml<int>(U"button_space");
		const auto buttonPadding = getToml<Point>(U"button_padding");

		auto&& exitRect = Rect(
			Scene::Size().moveBy(buttonPadding - buttonSize).movedBy(0, -buttonSpace * index),
			buttonSize);
		const bool exitHover =
			exitRect.intersects(RectF(Arg::center = Cursor::PosF(), Constants::CursorSize_64));
		const bool focused = Gm::IsUsingGamepad()
			                     ? index == cursorIndex
			                     : exitHover;
		drawButton(exitRect, focused);
		(void)FontAsset(AssetKeys::RocknRoll_24_Bitmap)(label).drawAt(exitRect.center());

		// 入力チェック
		if (focused)
		{
			if (Gm::IsUsingGamepad())
			{
				DrawButtonFrame(exitRect);
				if (IsGamepadDown(Gm::GamepadButton::A)) action();
			}
			else
			{
				if (MouseL.down()) action();
			}
		}

		return exitHover;
	}
}
