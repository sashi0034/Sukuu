#pragma once
#include "Play/Player.h"
#include "Play/Gimmick/GimmickGrid.h"

namespace Play
{
	class IEnemyInternal
	{
	public:
		virtual ~IEnemyInternal() = default;
		virtual Vec2 GetDrawPos() const = 0;
		virtual TextureRegion GetTexture() const = 0;
	};

	struct EnemyTransform;

	bool CanEnemyMoveTo(
		const MapGrid& map, const GimmickGrid& gimmick, const CharaVec2& currentActualPos, Dir4Type dir);

	void CheckSendEnemyCollide(Player& player, const EnemyTransform& transform, EnemyKind enemy);

	bool IsEnemyCollided(const EnemyTransform& transform, const RectF& collider);

	void PerformEnemyDestroyed(const IEnemyInternal& enemy);

	bool FaceEnemyMovableDir(
		Dir4Type& dir, const CharaPosition& pos, const MapGrid& map, const GimmickGrid& gimmick, bool leftPriority);

	bool RotateEnemyDirFacingPathway(Dir4Type& dir, const CharaPosition& pos, const MapGrid& map, bool leftPriority);

	bool RotateEnemyDirFacingMovable(
		Dir4Type& dir, const CharaPosition& pos, const MapGrid& map, const GimmickGrid& gimmick, bool leftPriority);

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

	enum class EnemyTrappedState
	{
		Normal,
		Killed,
		Captured,
	};

	struct EnemyTransform : IEnemyInternal
	{
		CharaPosition m_pos{};
		Vec2 m_animOffset{};
		bool m_collideEnabled{true};
		Dir4Type m_dir{Dir4::Down};
		AnimTimer m_animTimer{};
		EnemyTrappedState m_trapped{};
		EnemyPlayerTracker m_playerTracker{};
	};

	bool CheckEnemyTrappingGimmick(
		YieldExtended& yield,
		ActorView self,
		const Point& currentPoint, EnemyTransform& transform);
}
