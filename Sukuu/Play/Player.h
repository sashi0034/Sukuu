#pragma once

#include "Forward.h"
#include "Chara/CharaUtil.h"
#include "Enemy/EnemyDefinition.h"
#include "Player_detail/PlayerDefinition.h"

namespace Play
{
	class Player : public ActorBase
	{
	public:
		Player();
		void Init(const PlayerPersonalData& data, const Vec2& initialPos);
		void Update() override;
		double OrderPriority() const override;

		bool SendEnemyCollide(const RectF& rect, EnemyKind enemy);

		bool CanUseItem(int itemIndex) const;
		void RequestUseItem(int itemIndex);

		void PerformTutorialOpening();

		const PlayerPersonalData& PersonalData() const;
		Mat3x2 CameraTransform() const;
		const CharaPosition& CurrentPos() const;
		Point CurrentPoint() const;
		Vec2 GetActualViewPos() const;
		const PlayerDistField& DistField() const;
		bool IsSlowMotion() const;
		bool IsImmortal() const;
		bool IsTerminated() const;
		const PlayerVisionState& Vision() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
