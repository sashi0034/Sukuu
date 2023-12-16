#include "stdafx.h"
#include "DialogSettingConfigure.h"

#include "AssetKeys.h"
#include "Assets.generated.h"
#include "DialogGamepadRegister.h"
#include "GameConfig.h"
#include "GamepadObserver.h"
#include "detail/GameDialogCommon.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	using namespace Gm;
	using namespace Gm::detail;

	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"gm.setting_configure." + key);
	}

	constexpr int rectRound_8 = 8;

	struct InternalState
	{
		GameConfig editing;
		double passedStarted{};
		double passedFinished{};
	};

	void updateSideTriangle(const RectF& rightRect, bool entered, const std::function<void(bool right)>& onSubmit)
	{
		const auto submit = [&](bool right)
		{
			AudioAsset(AssetSes::system_yes).playOneShot();
			onSubmit(right);
		};

		// 三角形部分
		bool enteredLeft{};
		bool enteredRight{};
		if (entered)
		{
			if (IsUsingGamepad())
			{
				if (IsGamepadDown(GamepadButton::DLeft)) submit(false);
				else if (IsGamepadDown(GamepadButton::DRight)) submit(true);
				enteredRight = true;
				enteredLeft = true;
			}
			else
			{
				const bool aimingRight = Cursor::Pos().x > rightRect.center().x;
				if (MouseL.down()) submit(aimingRight);
				if (aimingRight) enteredRight = true;
				else enteredLeft = true;
			}
		}
		const int triangleSpace = getToml<int>(U"triangle_space");
		const int triangleSize = getToml<int>(U"triangle_size");
		(void)Triangle(rightRect.leftCenter().movedBy(triangleSpace, 0), triangleSize, -90_deg)
			.draw(enteredLeft ? DlRed : DlGray);
		(void)Triangle(rightRect.rightCenter().movedBy(-triangleSpace, 0), triangleSize, 90_deg)
			.draw(enteredRight ? DlRed : DlGray);
	}

	void drawContentRegionRect(const RectF& rightRect, bool entered)
	{
		(void)rightRect.stretched(entered ? 4 : 0)
		               .rounded(rectRound_8)
		               .drawShadow(Vec2{6, 6}, 12, 3, ColorF(DlGray, 0.5))
		               .draw(ColorF(1))
		               .drawFrame(1.5, DlBlack);
	}

	void drawContentTextRegion(
		const RectF& rightRect, bool entered, StringView name, const ColorF& focusedColor = DlBlack)
	{
		// コンテント領域
		drawContentRegionRect(rightRect, entered);
		FontAsset(AssetKeys::RocknRoll_Sdf)(name)
			.draw(DlFontSize(), Arg::center = rightRect.center(), entered ? focusedColor : DlGray);
	}

	void drawContentVolumeRegion(const RectF& rightRect, bool entered, double rate, double defaultRate)
	{
		// コンテント領域
		drawContentRegionRect(rightRect, entered);
		const auto c = entered ? DlBlack : DlGray;
		const auto r = rightRect.stretched(-64, -16)
		                        .drawFrame(2, c)
		                        .stretched(-8);
		(void)r.stretched(2, -r.w * (1.0 - defaultRate), 2, 0).right().draw(c);
		(void)r.stretched(0, -r.w * (1.0 - rate), 0, 0).draw(c);
	}

	using ContentUpdateFunc = std::function<void(const RectF& rightRect, bool entered)>;

	ContentUpdateFunc wrapTextContentWithTabs(StringView name, const std::function<void(bool right)>& onSubmit)
	{
		// 左右つまみ付きテキストコンテント領域
		return [name, onSubmit = std::move(onSubmit)](const RectF& rightRect, bool entered)
		{
			drawContentTextRegion(rightRect, entered, name);

			updateSideTriangle(rightRect, entered, onSubmit);
		};
	}

	ContentUpdateFunc wrapVolumeContentWithTabs(
		double rate, double defaultRate, const std::function<void(bool right)>& onSubmit)
	{
		// 左右つまみ付きボリュームコンテント領域
		return [rate,defaultRate, onSubmit = std::move(onSubmit)](const RectF& rightRect, bool entered)
		{
			drawContentVolumeRegion(rightRect, entered, rate, defaultRate);

			updateSideTriangle(rightRect, entered, onSubmit);
		};
	}

	ContentUpdateFunc wrapTextContentWithAction(StringView name, const std::function<void()>& onSubmit)
	{
		// 左右つまみ付きテキストコンテント領域
		return [name, onSubmit = std::move(onSubmit)](const RectF& rightRect, bool entered)
		{
			drawContentTextRegion(rightRect, entered, name, DlRed);

			if (not entered) return;

			const auto submit = [&]()
			{
				AudioAsset(AssetSes::system_yes).playOneShot();
				onSubmit();
			};

			// アクション実行
			if (IsUsingGamepad())
			{
				if (IsGamepadDown(GamepadButton::A)) submit();
			}
			else
			{
				if (MouseL.down()) submit();
			}
		};
	}

	void updateRow(int row, int* cursorRaw, StringView desc, const ContentUpdateFunc& contentUpdate)
	{
		const int top = getToml<int>(U"cell_top");
		const auto cellSize = getToml<Size>(U"cell_rect");
		const auto cellSpace = getToml<Point>(U"cell_space");
		const auto center = Vec2(Scene::Center().x, top + (cellSize.y + cellSpace.y) * row);

		const auto leftRect = RectF(Arg::rightCenter = center.movedBy(-cellSpace.x / 2, 0), cellSize);
		const auto rightRect = RectF(Arg::leftCenter = center.movedBy(cellSpace.x / 2, 0), cellSize);

		const bool entered = IsUsingGamepad()
			                     ? row == *cursorRaw
			                     : Cursor::Pos().intersects(rightRect);

		if (not desc.empty())
		{
			// 説明領域
			if (entered) (void)leftRect.rounded(rectRound_8).drawFrame(3, DlLightGray);
			else (void)leftRect.rounded(rectRound_8).draw(DlLightGray);
			const auto decsPos = leftRect.leftCenter().movedBy(getToml<int>(U"desc_left"), 0);
			FontAsset(AssetKeys::RocknRoll_Sdf)(desc)
				.draw(DlFontSize(), Arg::leftCenter = decsPos, DlBlack);
		}

		contentUpdate(rightRect, entered);
	}

	StringView getFullscreenName(bool fullscreen)
	{
		return fullscreen ? U"フルスクリーン" : U"ウィンドウ";
	}

	void updateUi(GameConfig& editing, bool* exitHover, int* cursorRaw)
	{
		static const auto defaultConfig = GameConfig();

		DrawDialogTitle(U"設定");

		updateRow(0, cursorRaw, U"画面モード", wrapTextContentWithTabs(
			          getFullscreenName(editing.fullscreen), [&](bool right)
			          {
				          editing.fullscreen = not editing.fullscreen;
				          editing.ApplySystems();
			          }));
		updateRow(1, cursorRaw, U"言語", wrapTextContentWithTabs(
			          LanguageLabels[static_cast<int>(editing.language)],
			          [&](bool right)
			          {
				          editing.language = Util::AddModuloEnum<
					          GameLanguage, GameLanguage::Max>(
					          editing.language, right ? 1 : -1);
				          editing.ApplySystems();
			          }));
		updateRow(2, cursorRaw, U"BGM", wrapVolumeContentWithTabs(
			          editing.bgm_volume.GetRate(),
			          defaultConfig.bgm_volume.GetRate(),
			          [&](bool right)
			          {
				          editing.bgm_volume = TenStepNumber(editing.bgm_volume + (right ? 1 : -1));
				          editing.ApplySystems();
			          }));
		updateRow(3, cursorRaw, U"SE", wrapVolumeContentWithTabs(
			          editing.se_volume.GetRate(),
			          defaultConfig.se_volume.GetRate(),
			          [&](bool right)
			          {
				          editing.se_volume = TenStepNumber(editing.se_volume + (right ? 1 : -1));
				          editing.ApplySystems();
			          }));
		updateRow(4, cursorRaw, U"カメラ移動量", wrapVolumeContentWithTabs(
			          editing.camera_move.GetRate(),
			          defaultConfig.camera_move.GetRate(),
			          [&](bool right)
			          {
				          editing.camera_move = TenStepNumber(editing.camera_move + (right ? 1 : -1));
			          }));
		updateRow(5, cursorRaw, U"", wrapTextContentWithAction(U"ゲームパッド設定", [&]
		{
			const auto gamepad = Gamepad(GamepadPlayer_0);
			if (not gamepad) return;
			if (const auto registered = DialogGamepadRegister())
				editing.gamepad_mapping[gamepad.getInfo().name] = registered.value();
		}));

		if (IsUsingGamepad())
		{
			*cursorRaw =
				Util::Mod2(*cursorRaw + (IsGamepadDown(GamepadButton::DDown) - IsGamepadDown(GamepadButton::DUp)),
				           5 + 1);
		}

		DrawDialogBottomLine();

		DrawDialogExit(exitHover);
	}

	GameConfig loopInternal(const GameConfig& current)
	{
		InternalState state{};
		state.editing = current;

		bool exitRequested{};
		int cursorRaw{};

		while (System::Update())
		{
			Transformer2D transformer2D{PreUpdateDialog(state.passedStarted, state.passedFinished, true)};

			state.passedStarted += Scene::DeltaTime();
			if (exitRequested) state.passedFinished += Scene::DeltaTime();

			bool exitHover;
			updateUi(state.editing, &exitHover, &cursorRaw);

			exitRequested = exitRequested || CheckDialogExit(exitHover, true);

			if (IsDialogFinishEnd(state.passedFinished)) break;
		}

		return state.editing;
	}
}

GameConfig Gm::DialogSettingConfigure(const GameConfig& current)
{
	const ScopedRenderTarget2D rt{none};

	const auto bg = Util::ScopedBackgroundStore();
	const auto result = loopInternal(current);
	return result;
}

void Gm::DialogSettingConfigure()
{
	GameConfig::Instance() = DialogSettingConfigure(GameConfig::Instance());
	GameConfig::Instance().RequestWrite();
}
