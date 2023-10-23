#pragma once

#include "PlayerDistField.h"

namespace Play
{
	constexpr int maxDist = 11;

	class PlayerDistFieldInternal::Impl
	{
	public:
		static void clearOld(PlayerDistFieldInternal& distField)
		{
			for (auto&& p : distField.m_wroteHistory)
			{
				distField.m_field[p] = {};
			}
			distField.m_wroteHistory.clear();
		}

		static void refreshInternal(
			PlayerDistFieldInternal& distField, const MapGrid& map, const Point& pos, Array<Point>& todoQueue)
		{
			const int currDist = distField.m_field[pos].distance;
			if (currDist >= maxDist) return;
			for (int i = 0; i < 4; ++i)
			{
				auto dir = Dir4Type(i);
				if (CanMovePointTo(map, pos, dir))
				{
					const auto checkingPos = pos + dir.ToXY().asPoint();
					auto& f = distField.m_field[checkingPos];
					if (f.distance <= currDist + 1) continue;
					f.distance = currDist + 1;
					distField.m_wroteHistory.push_back(checkingPos);
					todoQueue.push_back(checkingPos);
				}
			}
		}
	};

	void PlayerDistFieldInternal::Refresh(const MapGrid& map, const CharaVec2& playerPos)
	{
		// 前回の書き込みをクリア
		Impl::clearOld(*this);

		const Point playerPoint = playerPos.MapPoint();

		m_field[playerPoint] = {.distance = 0};
		if (CanMovePointAt(map, playerPoint) == false) return;

		static Array<Point> todoQueue{};
		todoQueue.clear();
		m_wroteHistory.push_back(playerPoint);
		Impl::refreshInternal(*this, map, playerPoint, todoQueue);

		// キューがなくなるまで更新
		while (todoQueue.size() > 0)
		{
			auto nextPos = todoQueue.back();
			todoQueue.pop_back();
			Impl::refreshInternal(*this, map, nextPos, todoQueue);
		}

		// 十字方向の部分はフラグを付ける
		for (int d = 0; d < 4; ++d)
		{
			auto dir = Dir4Type(d);
			for (int x = 0; x < maxDist; ++x)
			{
				auto& checking = m_field[playerPoint + dir.ToXY().asPoint() * x];
				if (checking.distance == PlayerDistanceInfinity) break;
				checking.directStraight = true;
			}
		}
	}
}
