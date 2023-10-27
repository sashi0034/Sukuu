#pragma once
#include "Play/Player.h"

namespace Play
{
	void CheckSendEnemyCollide(Player& player, CharaPosition& pos);

	bool IsEnemyCollided(const CharaPosition& pos, const RectF& collider);

	bool FaceEnemyMovableDir(Dir4Type& dir, const CharaPosition& pos, const MapGrid& map, bool leftPriority);

	bool RotateEnemyDirFacingPathway(Dir4Type& dir, const CharaPosition& pos, const MapGrid& map, bool leftPriority);

	bool RotateEnemyDirFacingMovable(Dir4Type& dir, const CharaPosition& pos, const MapGrid& map, bool leftPriority);

	class EnemyPlayerTracker
	{
	public:
		void Track(Dir4Type& direction,
		           const Point& currentPoint,
		           int maxConcern,
		           const std::function<void()>& onLostPlayer);
		int GetConcern() const { return m_concern; };
		bool IsTracking() const { return m_concern > 0; }

	private:
		int m_concern = 0;
	};
}
