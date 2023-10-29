# include "stdafx.h"
#include <Siv3D/Windows/Windows.hpp>

#include <iso646.h>

namespace
{
	constexpr Size mapSize{65, 65};

	void changeCharBy(Grid<char32_t>& cells, Point p, bool mirror, char32_t c)
	{
		if (cells.inBounds(p) == false) return;
		cells[p] = c;
		if (mirror) changeCharBy(cells, {cells.size().x - 1 - p.x, p.y}, false, c);
	}

	void changeAt(Grid<char32_t>& cells, Point p, bool mirror)
	{
		if (KeySpace.pressed()) changeCharBy(cells, p, mirror, U' ');
		if (KeyMinus.pressed()) changeCharBy(cells, p, mirror, U'-');
		if (KeyA.pressed()) changeCharBy(cells, p, mirror, U'A');
	}

	LONG_PTR g_baseProc = 0;
	bool g_receivedMessage{};

	LRESULT CALLBACK CustomWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		g_receivedMessage = true;
		return CallWindowProc(reinterpret_cast<WNDPROC>(g_baseProc), hWnd, message, wParam, lParam);
	}
}

void Main()
{
	// 自前ウィンドウ処理
	const auto hWnd = static_cast<HWND>(s3d::Platform::Windows::Window::GetHWND());
	g_baseProc = ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(::CustomWindowProc));

	Scene::SetBackground(ColorF{0.1});

	Window::SetStyle(WindowStyle::Sizable);
	Scene::SetResizeMode(ResizeMode::Keep);

	constexpr int32 cellSize = 10;
	const Font font(FontMethod::SDF, cellSize, Typeface::Bold);

	Camera2D camera2D{};
	Grid<char32_t> cellGrid(mapSize, U' ');

	size_t writeScaleIndex = 0;
	bool mirrored{};
	char32_t lastChar = U' ';
	bool dragging{};
	Point dragStart{};
	int sleepingCount{};

	while (System::Update())
	{
		while (true)
		{
			if (MouseR.pressed() || g_receivedMessage)
			{
				sleepingCount = 0;
				break;
			}
			sleepingCount++;
			if (sleepingCount < 20) break;
			System::Sleep(1000 / 20.0); // 20FPS
		}
		g_receivedMessage = false;
		// Console.writeln(Scene::Time());

		[&]
		{
			// 内容更新
			camera2D.update();
			const Transformer2D t = camera2D.createTransformer();
			DrawableText posMessage{};
			Point posPoint{};

			for (auto p : step(cellGrid.size()))
			{
				const Rect rect{(p * cellSize), cellSize};
				const bool isCenterLine = p.x == cellGrid.size().x / 2 || p.y == cellGrid.size().y / 2;
				(void)rect.stretched(-1).draw(isCenterLine ? ColorF(0.25, 0.25, 0.2) : ColorF{0.2});
				(void)font(cellGrid[p]).drawAt(rect.center(), Palette::White);

				if (rect.intersects(Cursor::Pos()) == false) continue;
				posMessage = font(Format(p));
				posPoint = rect.pos;

				const auto before = cellGrid[p];
				for (auto offset : step(Size{1, 1} * static_cast<int>(1 + writeScaleIndex * 2)))
				{
					// 普通に書き換え
					changeAt(cellGrid, p.movedBy(offset - Size{1, 1} * static_cast<int>(writeScaleIndex)), mirrored);
				}
				if (before != cellGrid[p]) lastChar = cellGrid[p];

				// 塗りつぶし処理
				if (MouseL.down())
				{
					dragging = true;
					dragStart = p;
				}
				else if (dragging && MouseL.pressed())
				{
					Rect(dragStart * cellSize, (p - dragStart + Point{1, 1}) * cellSize)
						.draw(ColorF{Palette::Yellow, 0.3});
				}
				else if (dragging && not MouseL.pressed())
				{
					dragging = false;
					// 塗りつぶし実行
					const Point tl{std::min(dragStart.x, p.x), std::min(dragStart.y, p.y)};
					const Point br{std::max(dragStart.x, p.x), std::max(dragStart.y, p.y)};
					for (const auto offset : step(br - tl + Point{1, 1}))
					{
						changeCharBy(cellGrid, tl.movedBy(offset), mirrored, lastChar);
					}
				}
			}
			// 位置情報
			(void)posMessage.draw(posPoint, Palette::Yellow);
		}();

		if (SimpleGUI::Button(U"Copy", Vec2{10, 10}))
		{
			String s{};
			for (int y = 0; y < cellGrid.size().y; ++y)
			{
				s += U"U\"";
				for (int x = 0; x < cellGrid.size().x; ++x)
				{
					s += cellGrid[y][x];
				}
				s += U"\",\n";
			}
			Clipboard::SetText(s);
		}

		if (SimpleGUI::Button(U"Paste", Vec2{110, 10}))
		{
			String s;
			Clipboard::GetText(s);
			int x{}, y{};
			bool inner{};
			for (const auto c : s)
			{
				if (c == U'\"')
				{
					if (inner)
					{
						x = 0;
						y++;
					}
					inner = not inner;
					continue;
				}
				if (not inner) continue;
				if (cellGrid.inBounds({x, y})) cellGrid[{x, y}] = c;
				x += 1;
			}
		}

		SimpleGUI::CheckBox(mirrored, U"Mirror", {10, 50});

		SimpleGUI::RadioButtons(writeScaleIndex, {U"Small", U"Medium", U"Large"}, {10, 90});
	}
}
