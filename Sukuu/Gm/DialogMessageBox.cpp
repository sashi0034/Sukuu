#include "stdafx.h"
#include "DialogMessageBox.h"

#include "AssetKeys.h"
#include "Assets.generated.h"
#include "GamepadObserver.h"
#include "detail/GameDialogCommon.h"
#include "Util/TomlParametersWrapper.h"
#include "Util/Utilities.h"

namespace
{
	using namespace Gm;
	using namespace Gm::detail;

	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"gm.yesno." + key);
	}

	enum class MessageBoxKind
	{
		YesNo,
		Ok,
	};

	enum class OptionalResponse
	{
		Unopt,
		Yes,
		No,
		Ok,
	};

	struct InternalState
	{
		double passedStarted{};
		double passedFinished{};
	};

	constexpr double textScale = 1.3;

	void updateYesNoButton(
		const Rect& rect,
		StringView text,
		const InputGroup& alt,
		OptionalResponse answer,
		OptionalResponse* confirmed,
		bool allowedConfirm)
	{
		const bool intersects = not IsUsingGamepad() && rect.intersects(Cursor::Pos());
		const bool entered = (intersects) || alt.pressed();
		const bool submitted = (intersects && MouseL.up()) || alt.up();;
		if (allowedConfirm && submitted && *confirmed == OptionalResponse::Unopt)
		{
			// 決定
			*confirmed = answer;
			if (answer == OptionalResponse::No) AudioAsset(AssetSes::system_no).playOneShot();
			if (answer == OptionalResponse::Yes) AudioAsset(AssetSes::system_yes).playOneShot();
		}
		(void)rect.rounded(8).draw((*confirmed == answer)
			                           ? DlRed
			                           : (entered ? (DlGray * 0.9) : DlGray));
		FontAsset(AssetKeys::RocknRoll_Sdf)(text).drawAt(DlFontSize() * textScale, rect.center(), ColorF(1));
	}

	template <MessageBoxKind messageBox>
	void updateUi(StringView message, OptionalResponse* confirmed, bool allowedConfirm)
	{
		DrawDialogTitle(U"確認");

		const int areaWidth = getToml<int>(U"area_width");
		(void)Line(Scene::Center().moveBy(-areaWidth / 2, 0), Scene::Center().moveBy(areaWidth / 2, 0))
			.draw(4, DlGray);

		const auto font = FontAsset(AssetKeys::RocknRoll_Sdf);
		font(message)
			.drawAt(DlFontSize() * textScale, Scene::Center().moveBy(0, getToml<int>(U"message_y")), DlBlack);

		const auto buttonSize = getToml<Size>(U"button_size");
		const auto buttonOffset = getToml<Point>(U"button_offset");

		if (messageBox == MessageBoxKind::YesNo)
		{
			const auto noRect = Rect(Arg::center = Scene::Center().moveBy(-buttonOffset.x, buttonOffset.y), buttonSize);
			const auto yesRect = Rect(Arg::center = Scene::Center().moveBy(buttonOffset), buttonSize);

			// いいえ
			updateYesNoButton(
				noRect,
				U"いいえ {}"_fmt(IsUsingGamepad() ? U"[B]" : U"(N)"),
				KeyN | GetGamepadInput(GamepadButton::B),
				OptionalResponse::No,
				confirmed,
				allowedConfirm);

			// はい
			updateYesNoButton(
				yesRect,
				U"はい {}"_fmt(IsUsingGamepad() ? U"[A]" : U"(Y)"),
				KeyY | GetGamepadInput(GamepadButton::A),
				OptionalResponse::Yes,
				confirmed,
				allowedConfirm);
		}
		else if (messageBox == MessageBoxKind::Ok)
		{
			// OK
			const auto okRect = Rect(Arg::center = Scene::Center().moveBy(0, buttonOffset.y), buttonSize);
			updateYesNoButton(
				okRect,
				U"了解 {}"_fmt(IsUsingGamepad() ? U"[A]" : U"(Y)"),
				KeyY | GetGamepadInput(GamepadButton::A),
				OptionalResponse::Ok,
				confirmed,
				allowedConfirm);
		}

		DrawDialogBottomLine();
	}

	template <MessageBoxKind messageBox>
	MessageBoxResult loopInternal(StringView message)
	{
		InternalState state{};
		OptionalResponse confirmed{};

		while (System::Update())
		{
			Transformer2D transformer2D{PreUpdateDialog(state.passedStarted, state.passedFinished, true)};

			state.passedStarted += Scene::DeltaTime();
			if (confirmed != OptionalResponse::Unopt) state.passedFinished += Scene::DeltaTime();

			updateUi<messageBox>(message, &confirmed, IsDialogStartEnd(state.passedStarted));

			if (IsDialogFinishEnd(state.passedFinished)) break;
		}

		if (confirmed == OptionalResponse::Yes) return MessageBoxResult::Yes;
		return MessageBoxResult::No;
	}
}

MessageBoxResult Gm::DialogYesNo(StringView message)
{
	const ScopedRenderTarget2D rt{none};

	const auto bg = Util::ScopedBackgroundStore();
	const auto result = loopInternal<MessageBoxKind::YesNo>(message);
	return result;
}

MessageBoxResult Gm::DialogOk(StringView message)
{
	const ScopedRenderTarget2D rt{none};

	const auto bg = Util::ScopedBackgroundStore();
	const auto result = loopInternal<MessageBoxKind::Ok>(message);
	return result;
}
