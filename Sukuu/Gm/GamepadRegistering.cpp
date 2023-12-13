#include "stdafx.h"
#include "GamepadRegistering.h"

#include "AssetKeys.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	using namespace Gm;

	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"gm.gamepad_registering." + key);
	}

	enum class RegisterStage
	{
		Register_A,
		Register_B,
		Register_X,
		Register_Y,
		Register_DRight,
		Register_DUp,
		Register_DLeft,
		Register_DDown,
		Register_LB,
		Register_RB,
		Register_LT,
		Register_RT,
		Register_Menu,
		Finished,
	};

	struct InternalState
	{
		Texture keyboardEmoji{U"⌨"_emoji};
		RegisterStage stage = RegisterStage::Register_A;
		std::array<int, GamepadButtonSize> registered{};
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

	void drawBeanButton(bool isEnabled, const String& bp)
	{
		if (isEnabled)
		{
			(void)Circle(getToml<Point>(bp), getToml<int>(U"bs_bean")).draw(redColor);
		}
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

	void drawGamepad(const InternalState& state)
	{
		const auto stage = state.stage;

		const double scale = getToml<double>(U"gamepad_scale");
		Transformer2D transformer2D{
			Mat3x2::Scale(scale, Scene::Center())
			.translated(0, getToml<int>(U"gamepad_y") * scale)
		};
		TextureAsset(AssetKeys::gamepad).resized(1024).drawAt(Scene::Center());

		using enum RegisterStage;
		drawCircleButton(stage == Register_A, U"bp_a", U"A");
		drawCircleButton(stage == Register_B, U"bp_b", U"B");
		drawCircleButton(stage == Register_X, U"bp_x", U"X");
		drawCircleButton(stage == Register_Y, U"bp_y", U"Y");

		drawSquareButton(stage == Register_DUp, U"bp_du");
		drawSquareButton(stage == Register_DLeft, U"bp_dl");
		drawSquareButton(stage == Register_DDown, U"bp_dd");
		drawSquareButton(stage == Register_DRight, U"bp_dr");

		drawHorizontalButton(stage == Register_LB, U"bp_lb", U"LB");
		drawHorizontalButton(stage == Register_RB, U"bp_rb", U"RB");

		drawVerticalButton(stage == Register_LT, U"bp_lt", U"LT");
		drawVerticalButton(stage == Register_RT, U"bp_rt", U"RT");

		drawBeanButton(stage == Register_Menu, U"bp_menu");
	}

	int getGamepadUpButton(const detail::Gamepad_impl& gamepad)
	{
		for (auto [i, button] : Indexed(gamepad.buttons))
		{
			if (button.up()) return i;
		}
		return -1;
	}

	void checkInput(InternalState& state, const detail::Gamepad_impl& gamepad)
	{
		if (KeyBackspace.down())
		{
			// 戻る
			state.stage = static_cast<RegisterStage>(std::max(0, (static_cast<int>(state.stage) - 1)));
		}

		const int downButton = getGamepadUpButton(gamepad);
		if (downButton != -1)
		{
			const auto r = [&](GamepadButton b)
			{
				// ボタンを割り当てる
				state.registered[static_cast<int>(b)] = downButton;

				// 進める
				state.stage = static_cast<RegisterStage>(static_cast<int>(state.stage) + 1);
			};

			switch (state.stage)
			{
			case RegisterStage::Register_A:
				r(GamepadButton::A);
				break;
			case RegisterStage::Register_B:
				r(GamepadButton::B);
				break;
			case RegisterStage::Register_X:
				r(GamepadButton::X);
				break;
			case RegisterStage::Register_Y:
				r(GamepadButton::Y);
				break;
			case RegisterStage::Register_DRight:
				r(GamepadButton::DRight);
				break;
			case RegisterStage::Register_DUp:
				r(GamepadButton::DUp);
				break;
			case RegisterStage::Register_DLeft:
				r(GamepadButton::DLeft);
				break;
			case RegisterStage::Register_DDown:
				r(GamepadButton::DDown);
				break;
			case RegisterStage::Register_LB:
				r(GamepadButton::LB);
				break;
			case RegisterStage::Register_RB:
				r(GamepadButton::RB);
				break;
			case RegisterStage::Register_LT:
				r(GamepadButton::LT);
				break;
			case RegisterStage::Register_RT:
				r(GamepadButton::RT);
				break;
			case RegisterStage::Register_Menu:
				r(GamepadButton::Menu);
				break;
			case RegisterStage::Finished:
				break;
			default: ;
			}
		}
	}

	void loopInternal()
	{
		const auto gamepad = Gamepad(0);
		if (not gamepad) return;

		InternalState state{};

		while (System::Update())
		{
#if _DEBUG
			Util::RefreshTomlParameters();
#endif
			Scene::SetBackground(getBgColor());

			// ClearPrint();
			// Print(Cursor::Pos());

			drawTexts(state);

			drawGamepad(state);

			checkInput(state, gamepad);

			if (state.stage == RegisterStage::Finished) break;
			if (KeyEscape.down()) break;
		}

		System::Update();
	}
}

void Gm::DialogGamepadRegistering()
{
	const auto beforeBg = Scene::GetBackground();
	loopInternal();
	Scene::SetBackground(beforeBg);
}
