#include "stdafx.h"
#include "MazeGenerator.h"

namespace Play
{
	class MazeGenInternal;

	namespace
	{
		struct MazeGenState
		{
			Grid<bool> dugFlag;
			Grid<bool> roomEntryFlag;
			Array<Rect> createdRooms;
		};
	}

	Point IndexToDirection(int index)
	{
		static constexpr std::array a{
			Point{1, 0},
			Point{0, 1},
			Point{-1, 0},
			Point{0, -1},
		};
		return a[index];
	}
}

class Play::MazeGenInternal
{
public:
	explicit MazeGenInternal(const MazeGenProps& props): props(props)
	{
	}

	void createRooms(MazeGenState& state) const
	{
		for (auto i : step(props.numRooms))
		{
			int x, y;
			while (true)
			{
				constexpr int padding = 3;
				x = padding + (Random(0, props.size.x - padding) / 2) * 2;
				y = padding + (Random(0, props.size.y - padding) / 2) * 2;
				if (canDigRoom(state, {{x - 2, y - 2}, {props.roomSize + 4, props.roomSize + 4}}))
					break;
			}

			// 部屋作成
			for (auto p : step({x, y}, {props.roomSize, props.roomSize}))
			{
				digAt(state, p);
			}
			state.createdRooms.push_back({{x, y}, {props.roomSize, props.roomSize}});

			if (Random(0, 1) == 0)
			{
				state.roomEntryFlag[{
					x + Random(0, props.roomSize / 2) * 2, y + (props.roomSize / 2) * 2 * Random(0, 1)
				}] = true;
			}
			else
			{
				state.roomEntryFlag[{
					x + (props.roomSize / 2) * 2 * Random(0, 1), y + Random(0, props.roomSize / 2) * 2
				}] = true;
			}
		}
	}

	void digPassageCompletely(MazeGenState& state) const
	{
		Point p1;
		while (true)
		{
			p1.x = 1 + Random(0, (props.size.x) / 2 - 1) * 2;
			p1.y = 1 + Random(0, (props.size.y) / 2 - 1) * 2;
			if (canDigAt(state, p1) == false)
			{
				// 点P1から掘れるだけ掘る
				while (true)
				{
					const auto p2 = digStroke(state, p1);
					if (p1 == p2) break;
					p1 = p2;
				}
			}
			if (hasDugCompletely(state)) break;
		}
	}

private:
	const MazeGenProps& props;

	bool canDigRoom(const MazeGenState& state, const Rect& rect) const
	{
		for (auto p : step(rect.pos, rect.size / 2, {2, 2}))
		{
			if (canDigAt(state, p) == false) return false;
		}
		return true;
	}

	bool isRangedPoint(const Point& p) const
	{
		return 0 <= p.x && p.x < props.size.x &&
			0 <= p.y && p.y < props.size.y;
	}

	bool canDigAt(const MazeGenState& state, const Point& p) const
	{
		if (isRangedPoint(p) == false) return false;
		return state.dugFlag[p] == false;
	}

	static void digAt(MazeGenState& state, const Point& p)
	{
		state.dugFlag[p] = true;
	}

	Point digStroke(MazeGenState& state, Point p1) const
	{
		const auto dig = [](MazeGenState& state, const Point& p1, const Point& d)
		{
			digAt(state, p1 + d * 1);
			digAt(state, p1 + d * 2);
			return p1 + d * 2;
		};
		Point p2 = p1;
		Array<int> dirs{0, 1, 2, 3};
		dirs.shuffle();

		for (int i = 0; i < dirs.size(); ++i)
		{
			const auto d = IndexToDirection(dirs[i]);
			if (isRangedPoint(p1 + d * 2) &&
				state.roomEntryFlag[p1 + d * 2] && state.dugFlag[p1 + d * 1] == false)
			{
				// 部屋作成時のフラグが存在していたら優先的に掘る
				return dig(state, p1, d);
			}
		}

		for (int i = 0; i < dirs.size(); ++i)
		{
			const auto d = IndexToDirection(dirs[i]);
			if (canDigAt(state, p1 + d * 2))
			{
				// 掘れる方向なら掘る
				return dig(state, p1, d);
			}
		}

		return p1;
	}

	bool hasDugCompletely(const MazeGenState& state) const
	{
		for (int x = 1; x < props.size.x; x += 2)
		{
			for (int y = 1; y < props.size.y; y += 2)
			{
				if (canDigAt(state, {x, y})) return false;
			}
		}
		return true;
	}
};

namespace Play
{
	MapGrid GenerateFreshMaze(const MazeGenProps& props)
	{
		const MazeGenInternal internal{props};
		MazeGenState state{
			.dugFlag = Grid<bool>{props.size},
			.roomEntryFlag = Grid<bool>{props.size},
			.createdRooms = {}
		};
		internal.createRooms(state);
		internal.digPassageCompletely(state);

		MapGrid grid{props.size};
		for (auto p : step({0, 0}, props.size))
		{
			if (state.dugFlag[p]) grid.At(p).kind = TerrainKind::Pathway;
		}
		for (auto&& room : state.createdRooms)
		{
			for (auto p : step(room.pos, room.size))
			{
				grid.At(p).kind = TerrainKind::Floor;
			}
		}
		grid.Rooms().swap(state.createdRooms);
		return grid;
	}
}
