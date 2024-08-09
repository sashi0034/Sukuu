#pragma once

#include "Forward.h"
#include "Chara/CharaUtil.h"
#include "Enemy/EnemyDefinition.h"
#include "Player_detail/PlayerDefinition.h"

namespace Play
{
	struct PlayerExportedService
	{
		bool forcedImmortal = false;
		bool canMove = true;
		bool canScoop = true;
		std::function<void(const CharaVec2& pos, bool isRunning)> onMoved{};
		std::function<void(const CharaVec2& pos)> onScooped{};
		std::function<bool(const CharaVec2& pos)> canMoveTo{};
		std::function<bool(const CharaVec2& pos)> canScoopTo{};
		Optional<Vec2> overrideCamera = none;
	};

	class Player : public ActorBase
	{
	public:
		Player();
		void Init(const PlayerPersonalData& data, const Vec2& initialPos);
		void Update() override;
		double OrderPriority() const override;

		void StartInitialCamara();

		bool SendEnemyCollide(const RectF& rect, EnemyKind enemy);

		bool CanUseItem(int itemIndex) const;
		void RequestUseItem(int itemIndex);

		void PerformTutorialOpening(double duration);

		const PlayerPersonalData& PersonalData() const;
		Mat3x2 CameraTransform() const;
		const CharaPosition& CurrentPos() const;
		Point CurrentPoint() const;
		Vec2 GetActualViewPos() const;
		const PlayerDistField& DistField() const;
		bool IsSlowMotion() const;
		bool IsImmortal() const;
		bool IsTerminated() const;
		bool HasAbducted() const;
		const PlayerVisionState& Vision() const;

		PlayerExportedService& ExportedService();

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
