#include "stdafx.h"
#include "GamepadRegistering.h"

#include "GamepadObserver.h"
#include "AssetKeys.h"
#include "Assets.generated.h"
#include "detail/GameDialogCommon.h"
#include "Util/TomlParametersWrapper.h"
#include "Util/Utilities.h"

namespace
{
	using namespace Gm;
	using namespace Gm::detail;

	using Gamepad_impl = s3d::detail::Gamepad_impl;

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

	constexpr std::array<StringView, static_cast<int>(RegisterStage::Finished) + 1> stageDescription = {
		U"[ A ] アイテム",
		U"[ B ] ダッシュ",
		U"[ X ] ?",
		U"[ Y ] ?",
		U"[ Right ] 移動",
		U"[ Up ] 移動",
		U"[ Left ] 移動",
		U"[ Down ] 移動",
		U"[ LB ] アイテム変更",
		U"[ RB ] アイテム変更",
		U"[ LT ] 方向転換",
		U"[ RT ] すくう",
		U"[ Menu ] ポーズ",
		U"終了"
	};

	struct InternalState
	{
		Texture keyboardEmoji{U"⌨"_emoji};
		RegisterStage stage = RegisterStage::Register_A;
		GamepadButtonMapping registered{};
		double passedStarted{};
		double passedFinished{};
		double passedCurrentStage{};
	};

	void drawTexts(const InternalState& state, const GamepadInfo& gamepad, bool* exitHover)
	{
		const auto fontSize = DlFontSize();

		DrawDialogTitle(U"コントローラー登録\n" + gamepad.name);

		const auto bottom = Vec2{Scene::Center().x, Scene::Size().y};
		const auto bottom1 = DlBottom1();

		DrawDialogExit(exitHover);

		const auto rollbackArea = FontAsset(AssetKeys::RocknRoll_Sdf)(U"Backspace やり直し").drawAt(
			fontSize, bottom1, DlBlack);
		FontAsset(AssetKeys::RocknRoll_Sdf)(U"設定したいボタンを押してください").drawAt(
			fontSize, bottom.movedBy(0, -getToml<int>(U"bottom2")), DlBlack);
		const auto buttDesc = FontAsset(AssetKeys::RocknRoll_Sdf)(stageDescription[static_cast<int>(state.stage)]);
		const auto bottom3 = bottom.movedBy(0, -getToml<int>(U"bottom3"));
		(void)buttDesc.regionAt(fontSize, bottom3)
		              .stretched(getToml<Point>(U"desc_padding"))
		              .rounded(10)
		              .drawShadow(Vec2{3, 3}, 12, 1).draw(DlGray)
		              .draw(state.stage == RegisterStage::Finished ? DlRed : DlGray);
		(void)buttDesc.drawAt(fontSize, bottom3, Palette::White);

		DrawDialogBottomLine();

		state.keyboardEmoji.resized(getToml<int>(U"keyboard_size"))
		     .draw(Arg::rightCenter = Vec2{rollbackArea.leftX() - 16, bottom1.y});
	}

	void drawButtonLiteral(bool isEnabled, const String& bp, const String& buttonLiteral)
	{
		(void)FontAsset(AssetKeys::RocknRoll_Sdf)(buttonLiteral)
			.drawAt(getToml<int>(U"bs_text"), getToml<Point>(bp), isEnabled ? ColorF(1) : DlBlack);
	}

	void drawCircleButton(bool isEnabled, const String& bp, const String& buttonLiteral)
	{
		if (isEnabled)
		{
			(void)Circle(getToml<Point>(bp), getToml<int>(U"bs_circle")).draw(DlRed);
		}
		drawButtonLiteral(isEnabled, bp, buttonLiteral);
	}

	void drawBeanButton(bool isEnabled, const String& bp)
	{
		if (isEnabled)
		{
			(void)Circle(getToml<Point>(bp), getToml<int>(U"bs_bean")).draw(DlRed);
		}
	}

	void drawSquareButton(bool isEnabled, const String& bp)
	{
		if (isEnabled)
			(void)Rect(Arg::center = getToml<Point>(bp), getToml<int>(U"bs_square")).rounded(8).draw(DlRed);
	}

	void drawHorizontalButton(bool isEnabled, const String& bp, const String& buttonLiteral)
	{
		if (isEnabled)
			(void)Rect(Arg::center = getToml<Point>(bp), getToml<Point>(U"bs_horizontal")).rounded(16).draw(DlRed);
		drawButtonLiteral(isEnabled, bp, buttonLiteral);
	}

	void drawVerticalButton(bool isEnabled, const String& bp, const String& buttonLiteral)
	{
		if (isEnabled)
			(void)Rect(Arg::center = getToml<Point>(bp), getToml<Point>(U"bs_vertical")).rounded(16).draw(DlRed);
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

	int getGamepadDownButton(const Gamepad_impl& gamepad)
	{
		for (auto [i, button] : Indexed(gamepad.buttons))
		{
			if (button.down()) return i;
		}
		return -1;
	}

	void checkInput(InternalState& state, const Gamepad_impl& gamepad)
	{
		const int downButton = getGamepadDownButton(gamepad);
		if (downButton != -1 && state.passedCurrentStage > 0.1)
		{
			const auto r = [&](GamepadButton b)
			{
				// ボタンを割り当てる
				state.registered[b] = downButton;

				// 進める
				state.stage = static_cast<RegisterStage>(static_cast<int>(state.stage) + 1);
				state.passedCurrentStage = 0;

				AudioAsset(AssetSes::system_yes).playOneShot();
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

	bool canFinishTransition(const InternalState& state)
	{
		return (state.stage == RegisterStage::Finished && state.passedCurrentStage > 1.5);
	}

	Optional<GamepadButtonMapping> loopInternal()
	{
		const auto gamepad = Gamepad(GamepadPlayer_0);
		if (not gamepad) return none;

		InternalState state{};
		bool cancelRequested{};

		while (System::Update())
		{
			Transformer2D transformer2D{PreUpdateDialog(state.passedStarted, state.passedFinished)};

			state.passedStarted += Scene::DeltaTime();
			state.passedCurrentStage += Scene::DeltaTime();
			if (canFinishTransition(state) || cancelRequested) state.passedFinished += Scene::DeltaTime();

			bool exitHover;
			drawTexts(state, gamepad.getInfo(), &exitHover);

			drawGamepad(state);

			checkInput(state, gamepad);

			// キャンセルチェック
			cancelRequested = cancelRequested || CheckDialogExit(exitHover);

			if (KeyBackspace.down())
			{
				// 戻る
				state.stage = static_cast<RegisterStage>(std::max(0, (static_cast<int>(state.stage) - 1)));
				AudioAsset(AssetSes::system_no).playOneShot();
			}

			if (IsFinishDialog(state.passedFinished)) break;
		}

		System::Update();
		if (cancelRequested) return none;
		return state.registered;
	}
}

Optional<GamepadButtonMapping> Gm::DialogGamepadRegistering()
{
	const auto bg = Util::ScopedBackgroundStore();
	auto result = loopInternal();
	return result;
}
