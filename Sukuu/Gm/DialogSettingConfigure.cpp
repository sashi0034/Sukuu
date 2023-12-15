#include "stdafx.h"
#include "DialogSettingConfigure.h"

#include "AssetKeys.h"
#include "GameConfig.h"
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

	struct InternalState
	{
		GameConfig editing;
		double passedStarted{};
		double passedFinished{};
	};

	void drawLine(int line, StringView desc, StringView content)
	{
		const int top = getToml<int>(U"cell_top");
		const auto cellSize = getToml<Size>(U"cell_rect");
		const auto cellSpace = getToml<Point>(U"cell_space");
		const auto center = Vec2(Scene::Center().x, top + (cellSize.y + cellSpace.y) * line);

		const auto leftRect = RectF(Arg::rightCenter = center.movedBy(-cellSpace.x / 2, 0), cellSize);
		const auto rightRect = RectF(Arg::leftCenter = center.movedBy(cellSpace.x / 2, 0), cellSize);

		constexpr int rectRound = 8;

		const bool entered = Cursor::Pos().intersects(rightRect);

		if (not desc.empty())
		{
			// 説明領域
			if (entered) (void)leftRect.rounded(rectRound).drawFrame(3, DlLightGray);
			else (void)leftRect.rounded(rectRound).draw(DlLightGray);
			const auto decsPos = leftRect.leftCenter().movedBy(getToml<int>(U"desc_left"), 0);
			FontAsset(AssetKeys::RocknRoll_Sdf)(desc)
				.draw(DlFontSize(), Arg::leftCenter = decsPos, DlBlack);
		}

		// コンテント領域
		(void)rightRect.stretched(entered ? 4 : 0)
		               .rounded(rectRound)
		               .drawShadow(Vec2{6, 6}, 12, 3, ColorF(DlGray, 0.5))
		               .draw(ColorF(1))
		               .drawFrame(1.5, DlBlack);
		FontAsset(AssetKeys::RocknRoll_Sdf)(content)
			.draw(DlFontSize(), Arg::center = rightRect.center(), entered ? DlBlack : DlGray);

		// 三角形描画
		bool enteredLeft{};
		bool enteredRight{};
		if (entered)
		{
			if (Cursor::Pos().x < rightRect.center().x) enteredLeft = true;
			else enteredRight = true;
		}
		const int triangleSpace = getToml<int>(U"triangle_space");
		const int triangleSize = getToml<int>(U"triangle_size");
		(void)Triangle(rightRect.leftCenter().movedBy(triangleSpace, 0), triangleSize, -90_deg)
			.draw(enteredLeft ? DlRed : DlGray);
		(void)Triangle(rightRect.rightCenter().movedBy(-triangleSpace, 0), triangleSize, 90_deg)
			.draw(enteredRight ? DlRed : DlGray);
	}

	void updateUi(GameConfig& editing, bool* exitHover)
	{
		DrawDialogTitle(U"設定");

		drawLine(0, U"画面モード", U"フルスクリーン");
		drawLine(1, U"言語", U"フルスクリーン");
		drawLine(2, U"BGM", U"");
		drawLine(3, U"SE", U"");
		drawLine(4, U"SE", U"");
		drawLine(5, U"カメラ移動量", U"フルスクリーン");
		drawLine(6, U"", U"ゲームパッド設定");

		DrawDialogBottomLine();

		DrawDialogExit(exitHover);
	}

	GameConfig loopInternal(const GameConfig& current)
	{
		InternalState state{};
		state.editing = current;

		bool exitRequested{};

		while (System::Update())
		{
			Transformer2D transformer2D{PreUpdateDialog(state.passedStarted, state.passedFinished, true)};

			state.passedStarted += Scene::DeltaTime();
			if (exitRequested) state.passedFinished += Scene::DeltaTime();

			bool exitHover;
			updateUi(state.editing, &exitHover);

			exitRequested = exitRequested || CheckDialogExit(exitHover, true);

			if (IsFinishDialog(state.passedFinished)) break;
		}

		return state.editing;
	}
}

GameConfig Gm::DialogSettingConfigure(const GameConfig& current)
{
	const auto bg = Util::ScopedBackgroundStore();
	const auto result = loopInternal(current);
	return result;
}
