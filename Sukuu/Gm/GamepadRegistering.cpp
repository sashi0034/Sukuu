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

	struct InternalState
	{
		Texture keyboardEmoji{U"⌨"_emoji};
	};

	constexpr ColorF blackColor = ColorF(0.3);
	constexpr ColorF grayColor = Palette::Gray;
	constexpr ColorF redColor = Palette::Red;

	int getFontSize()
	{
		return getToml<int>(U"font_size");
	}

	ColorF getBgColor()
	{
		return getToml<ColorF>(U"bg");
	}

	void drawTexts(const InternalState& state)
	{
		const auto fontSize = getFontSize();

		const auto titleText = FontAsset(AssetKeys::RocknRoll_24_Bitmap)(U"コントローラー登録画面");
		const auto titlePos = Vec2{1, 1} * getToml<int>(U"top");
		(void)titleText.region(Arg::leftCenter = titlePos).stretched(16, 8).draw(grayColor);
		titleText.draw(Arg::leftCenter = titlePos, ColorF(1.0));

		const auto bottom = Vec2{Scene::Center().x, Scene::Size().y};
		const auto bottom1 = bottom.movedBy(0, -getToml<int>(U"bottom1"));
		(void)FontAsset(AssetKeys::RocknRoll_Sdf)(U"Backspace 一つ戻る\tEsc キャンセルして戻る").drawAt(
			fontSize, bottom1, blackColor);
		FontAsset(AssetKeys::RocknRoll_Sdf)(U"設定したいボタンを押してください").drawAt(
			fontSize, bottom.movedBy(0, -getToml<int>(U"bottom2")), blackColor);
		const auto buttDesc = FontAsset(AssetKeys::RocknRoll_Sdf)(U"B BUTTON : DASH");
		const auto bottom3 = bottom.movedBy(0, -getToml<int>(U"bottom3"));
		(void)buttDesc.regionAt(fontSize, bottom3)
		              .stretched(getToml<Point>(U"desc_padding"))
		              .rounded(10)
		              .drawShadow(Vec2{3, 3}, 12, 1).draw(grayColor)
		              .draw(grayColor);
		(void)buttDesc.drawAt(fontSize, bottom3, Palette::White);

		constexpr int lineX = 64;
		const int lineY = bottom.y - getToml<int>(U"line_y");
		const int lineThickness = getToml<int>(U"line_thickness");
		(void)Line(lineX, lineY, Scene::Size().x - lineX, lineY).draw(lineThickness, grayColor);

		state.keyboardEmoji.resized(getToml<int>(U"keyboard_size"))
		     .draw(Arg::leftCenter = Vec2{getToml<int>(U"keyboard_left"), bottom1.y});
	}

	void drawButtonLiteral(bool isEnabled, const String& bp, const String& buttonLiteral)
	{
		(void)FontAsset(AssetKeys::RocknRoll_Sdf)(buttonLiteral)
			.drawAt(getToml<int>(U"bs_text"), getToml<Point>(bp), isEnabled ? ColorF(1) : blackColor);
	}

	void drawCircleButton(bool isEnabled, const String& bp, const String& buttonLiteral)
	{
		if (isEnabled)
		{
			(void)Circle(getToml<Point>(bp), getToml<int>(U"bs_circle")).draw(redColor);
		}
		drawButtonLiteral(isEnabled, bp, buttonLiteral);
	}

	void drawSquareButton(bool isEnabled, const String& bp)
	{
		if (isEnabled)
			(void)Rect(Arg::center = getToml<Point>(bp), getToml<int>(U"bs_square")).rounded(8).draw(redColor);
	}

	void drawHorizontalButton(bool isEnabled, const String& bp, const String& buttonLiteral)
	{
		if (isEnabled)
			(void)Rect(Arg::center = getToml<Point>(bp), getToml<Point>(U"bs_horizontal")).rounded(16).draw(redColor);
		drawButtonLiteral(isEnabled, bp, buttonLiteral);
	}

	void drawVerticalButton(bool isEnabled, const String& bp, const String& buttonLiteral)
	{
		if (isEnabled)
			(void)Rect(Arg::center = getToml<Point>(bp), getToml<Point>(U"bs_vertical")).rounded(16).draw(redColor);
		drawButtonLiteral(isEnabled, bp, buttonLiteral);
	}

	void updateGamepad()
	{
		const double scale = getToml<double>(U"gamepad_scale");
		Transformer2D transformer2D{
			Mat3x2::Scale(scale, Scene::Center())
			.translated(0, getToml<int>(U"gamepad_y") * scale)
		};
		TextureAsset(AssetKeys::gamepad).resized(1024).drawAt(Scene::Center());

		drawCircleButton(true, U"bp_a", U"A");
		drawCircleButton(true, U"bp_b", U"B");
		drawCircleButton(true, U"bp_x", U"X");
		drawCircleButton(true, U"bp_y", U"Y");

		drawSquareButton(true, U"bp_du");
		drawSquareButton(true, U"bp_dl");
		drawSquareButton(true, U"bp_dd");
		drawSquareButton(true, U"bp_dr");

		drawHorizontalButton(true, U"bp_lb", U"LB");
		drawHorizontalButton(true, U"bp_rb", U"RB");

		drawVerticalButton(true, U"bp_lt", U"LT");
		drawVerticalButton(true, U"bp_rt", U"RT");
	}

	void loopInternal()
	{
		InternalState state{};

		while (System::Update())
		{
			Util::RefreshTomlParameters();
			Scene::SetBackground(getBgColor());

			// ClearPrint();
			// Print(Cursor::Pos());

			drawTexts(state);

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
