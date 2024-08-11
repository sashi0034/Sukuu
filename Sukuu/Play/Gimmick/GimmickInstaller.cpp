#include "stdafx.h"
#include "GimmickInstaller.h"

#include "Constants.h"
#include "Util/Dir4.h"

namespace
{
	using namespace Play;

	void applyArrowFromDirection(GimmickGrid& gimmick, const Point p, Dir4Type dir, bool isDemi)
	{
		switch (dir)
		{
		case Dir4::Right:
			gimmick[p] = isDemi ? GimmickKind::DemiArrow_right : GimmickKind::Arrow_right;
			break;
		case Dir4::Up:
			gimmick[p] = isDemi ? GimmickKind::DemiArrow_up : GimmickKind::Arrow_up;
			break;
		case Dir4::Left:
			gimmick[p] = isDemi ? GimmickKind::DemiArrow_left : GimmickKind::Arrow_left;
			break;
		case Dir4::Down:
			gimmick[p] = isDemi ? GimmickKind::DemiArrow_down : GimmickKind::Arrow_down;
			break;
		default: break;
		}
	}
}

namespace Play
{
	void InstallArrowsInMaze(GimmickGrid& gimmick, const MapGrid& map)
	{
		for (const auto p : step({1, 1}, gimmick.size().moveBy(-2, -2)))
		{
			if (map.Data()[p].kind == TerrainKind::Wall) continue;;

			int wallCount = 0;
			Dir4Type movableDir{Dir4::Invalid};
			for (int dir = 0; dir < 4; ++dir)
			{
				if (map.Data()[p.movedBy(Dir4Type(dir).ToXY().asPoint())].kind == TerrainKind::Wall) wallCount++;
				else movableDir = dir;
			}
			if (wallCount != 3) continue;

			applyArrowFromDirection(gimmick, p, movableDir.Reversed(), false);
		}
	}

	void InstallArrowsInDungeon(GimmickGrid& gimmick, const MapGrid& map)
	{
		const int w0 = std::min(map.Data().size().x, map.Data().size().y);
		constexpr int w_81 = 81;
		if (w0 <= w_81) return;

		// 矢印の最大個数
		const int arrow = Random(0, 1 + (w0 - w_81) / 4);

		for (const auto a : step(arrow))
		{
			const auto dirs = Array<Dir4Type>{Dir4::Right, Dir4::Up, Dir4::Left, Dir4::Down}.shuffled();

			for (const auto i : step(Constants::BigValue_100000))
			{
				const Point r = map.Rooms().RandomRoomPoint(false);

				// 床でないなら弾く
				if (map.At(r).kind != TerrainKind::Floor) continue;

				// マップぎりぎりを弾く
				if (r.x <= 0 || r.y <= 0 || r.x >= map.Data().size().x - 1 || r.y >= map.Data().size().y - 1) continue;

				// 壁がある方向に向かって矢印を設置
				for (auto dir : dirs)
				{
					const auto directedPoint = r.movedBy(dir.ToXY().asPoint());
					if (map.At(directedPoint).kind != TerrainKind::Wall) continue;
					applyArrowFromDirection(gimmick, r, dir, true);
					break;
				}
				break;
			}
		}
	}

	void InstallTreesInDungeon(GimmickGrid& gimmick, const MapGrid& map)
	{
		for (int x = 1; x < gimmick.size().x; x += 2)
		{
			for (int y = 1; y < gimmick.size().y; y += 2)
			{
				if (map.At({x, y}).kind != TerrainKind::Wall) continue;

				const int c = (x % 4) + (y % 4);
				if (c == 2 || c == 6)
				{
					gimmick[y][x] = GimmickKind::Tree_large;
				}
				else
				{
					gimmick[y][x] = GimmickKind::Tree_small;
				}
			}
		}
	}

	bool InstallGimmickRandomly(GimmickGrid& gimmick, const MapGrid& map, GimmickKind kind)
	{
		int failedCount{};
		while (true)
		{
			const auto p = map.Rooms().RandomRoomPoint(true);
			if (gimmick[p] == GimmickKind::None)
			{
				gimmick[p] = kind;
				return true;
			}
			failedCount++;
			if (failedCount >= Constants::BigValue_1000) return false;
		}
	}
}
