#pragma once

#include "Forward.h"
#include "Chara/CharaUtil.h"
#include "Player_detail\PlayerDefinition.h"

namespace Play
{
	class Player : public ActorBase
	{
	public:
		Player();
		void Init(const PlayerPersonalData& data);
		void Update() override;
		double OrderPriority() const override;

		void SendEnemyCollide(const RectF& rect);
		bool RequestUseItem(int itemIndex);

		const PlayerPersonalData& PersonalData() const;
		Mat3x2 CameraTransform() const;
		const CharaPosition& CurrentPos() const;
		Point CurrentPoint() const;
		const PlayerDistField& DistField() const;
		bool IsCompletedGoal() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
