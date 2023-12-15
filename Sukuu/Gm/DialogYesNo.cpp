#include "stdafx.h"
#include "DialogYesNo.h"

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

	enum class OptionalYesNo
	{
		Unopt,
		Yes,
		No,
	};

	struct InternalState
	{
		double passedStarted{};
		double passedFinished{};
	};

	constexpr double textScale = 1.3;

	void updateYesNoButton(
		const Rect& rect, StringView text, const InputGroup& alt, OptionalYesNo answer, OptionalYesNo* confirmed)
	{
		const bool intersects = not IsUsingGamepad() && rect.intersects(Cursor::Pos());
		const bool entered = (intersects) || alt.pressed();
		const bool submitted = (intersects && MouseL.up()) || alt.up();;
		if (submitted && *confirmed == OptionalYesNo::Unopt)
		{
			// 決定
			*confirmed = answer;
			if (answer == OptionalYesNo::No) AudioAsset(AssetSes::system_no).playOneShot();
			if (answer == OptionalYesNo::Yes) AudioAsset(AssetSes::system_yes).playOneShot();
		}
		(void)rect.rounded(8).draw((*confirmed == answer)
			                           ? DlRed
			                           : (entered ? (DlGray * 0.9) : DlGray));
		FontAsset(AssetKeys::RocknRoll_Sdf)(text).drawAt(DlFontSize() * textScale, rect.center(), ColorF(1));
	}

	void updateUi(StringView message, OptionalYesNo* confirmed)
	{
		DrawDialogTitle(U"確認");

		const int areaWidth = getToml<int>(U"area_width");
		(void)Line(Scene::Center().moveBy(-areaWidth / 2, 0), Scene::Center().moveBy(areaWidth / 2, 0))
			.draw(4, DlGray);

		FontAsset(AssetKeys::RocknRoll_Sdf)(message)
			.drawAt(DlFontSize() * textScale, Scene::Center().moveBy(0, getToml<int>(U"message_y")), DlBlack);

		const auto buttonSize = getToml<Size>(U"button_size");
		const auto buttonOffset = getToml<Point>(U"button_offset");
		const auto noRect = Rect(Arg::center = Scene::Center().moveBy(-buttonOffset.x, buttonOffset.y), buttonSize);
		const auto yesRect = Rect(Arg::center = Scene::Center().moveBy(buttonOffset), buttonSize);

		// いいえ
		updateYesNoButton(
			noRect,
			U"いいえ {}"_fmt(IsUsingGamepad() ? U"[B]" : U"(N)"),
			KeyN | GetGamepadInput(GamepadButton::B),
			OptionalYesNo::No,
			confirmed);

		// はい
		updateYesNoButton(
			yesRect,
			U"はい {}"_fmt(IsUsingGamepad() ? U"[A]" : U"(Y)"),
			KeyY | GetGamepadInput(GamepadButton::A),
			OptionalYesNo::Yes,
			confirmed);

		DrawDialogBottomLine();
	}

	MessageBoxResult loopInternal(StringView message)
	{
		InternalState state{};
		OptionalYesNo confirmed{};

		while (System::Update())
		{
			Transformer2D transformer2D{PreUpdateDialog(state.passedStarted, state.passedFinished, true)};

			state.passedStarted += Scene::DeltaTime();
			if (confirmed != OptionalYesNo::Unopt) state.passedFinished += Scene::DeltaTime();

			updateUi(message, &confirmed);

			if (IsFinishDialog(state.passedFinished)) break;
		}
		return confirmed == OptionalYesNo::Yes ? MessageBoxResult::Yes : MessageBoxResult::No;
	}
}

MessageBoxResult Gm::DialogYesNo(StringView message)
{
	const ScopedRenderTarget2D rt{none};

	const auto bg = Util::ScopedBackgroundStore();
	const auto result = loopInternal(message);
	return result;
}
