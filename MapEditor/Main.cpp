# include <Siv3D.hpp>

void Main()
{
	Scene::SetBackground(Palette::White);

	constexpr int32 cellSize = 40;

	// シーンのサイズとセルの大きさから縦横のセルの個数を計算
	Grid<int32> cells(Scene::Size() / cellSize);

	while (System::Update())
	{
		// カーソルを手の形に
		Cursor::RequestStyle(CursorStyle::Hand);

		for (auto p : step(cells.size()))
		{
			const Rect rect{ (p * cellSize), cellSize };

			if (rect.leftClicked())
			{
				// 0 → 1 → 2 →　3 → 0 → 1 → ... と遷移させる
				++cells[p] %= 4;
			}

			rect.stretched(-1).draw(ColorF{ 0.95 - cells[p] * 0.3 });
		}
	}
}
