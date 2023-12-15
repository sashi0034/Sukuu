#include "stdafx.h"
#include "GameDialogCommon.h"

#include "AssetKeys.h"
#include "Assets.generated.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"gm.dialog_common." + key);
	}

	constexpr double transitionStart = 0.3;
	constexpr double transitionFinish = 0.3;
}

namespace Gm::detail
{
	int DlFontSize()
	{
		return getToml<int>(U"font_size");
	}

	ColorF DlBackground()
	{
		return getToml<ColorF>(U"bg");
	}

	Vec2 DlBottom1()
	{
		return Vec2{Scene::Center().x, Scene::Size().y}.movedBy(0, -getToml<int>(U"bottom1"));
	}

	Transformer2D PreUpdateDialog(double passedStarted, double passedFinished)
	{
#if _DEBUG
		Util::RefreshTomlParameters();
#endif
		Scene::SetBackground(DlBackground());

		// 簡易遷移アニメーション
		const double transitionScale = 0.5
			+ 0.5 * EaseOutBack(Math::Min(transitionStart, passedStarted) / transitionStart)
			- 0.5 * EaseInBack(Math::Min(transitionFinish, passedFinished) / transitionFinish);
		return Transformer2D{
			Mat3x2::Scale(transitionScale, Scene::Center()).rotated(ToRadians(45 * (1.0 - transitionScale)))
		};
	}

	bool IsFinishDialog(double passedFinished)
	{
		return passedFinished > transitionFinish;
	}

	void DrawDialogTitle(StringView title)
	{
		const auto titleText = FontAsset(AssetKeys::RocknRoll_24_Bitmap)(title);
		const auto titlePos = Vec2{1, 1} * getToml<int>(U"top");
		(void)titleText.region(Arg::leftCenter = titlePos).stretched(16, 8).draw(DlGray);
		titleText.draw(Arg::leftCenter = titlePos, ColorF(1.0));
	}

	void DrawDialogBottomLine()
	{
		constexpr int lineX = 64;
		const int lineY = Scene::Size().y - getToml<int>(U"line_y");
		const int lineThickness = getToml<int>(U"line_thickness");
		(void)Line(lineX, lineY, Scene::Size().x - lineX, lineY).draw(lineThickness, DlGray);
	}

	void DrawDialogExit(bool* exitHover)
	{
		const auto exitPoint = Vec2(getToml<int>(U"exit_left"), DlBottom1().y);
		const auto exitText = FontAsset(AssetKeys::RocknRoll_Sdf)(U"戻る");

		const int fontSize = DlFontSize();
		const auto exitRect = exitText.region(fontSize, Arg::leftCenter = exitPoint).stretched(32, 4);
		*exitHover = exitRect.intersects(Cursor::Pos());
		(void)exitRect.rounded(8).draw(DlGray * (*exitHover ? 0.7 : 1.0));
		(void)exitText.draw(fontSize, Arg::leftCenter = exitPoint, ColorF(1));
	}

	bool CheckDialogExit(bool exitHover)
	{
		if (KeyEscape.down() || (exitHover && MouseL.down()))
		{
			// キャンセル
			AudioAsset(AssetSes::system_no).playOneShot();
			return true;
		}
		return false;
	}
}
