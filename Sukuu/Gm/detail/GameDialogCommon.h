#pragma once

namespace Gm::detail
{
	constexpr ColorF DlBlack = ColorF(0.3);
	constexpr ColorF DlGray = Palette::Gray;
	constexpr ColorF DlRed = Palette::Red;

	int DlFontSize();
	ColorF DlBackground();
	Vec2 DlBottom1();

	Transformer2D PreUpdateDialog(double passedStarted, double passedFinished);
	bool IsFinishDialog(double passedFinished);
	void DrawDialogTitle(StringView title);
	void DrawDialogBottomLine();
	void DrawDialogExit(bool* exitHover);
	bool CheckDialogExit(bool exitHover);
}
