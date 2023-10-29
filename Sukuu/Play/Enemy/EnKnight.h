#pragma once

#include "EnemyDefinition.h"

namespace Play
{
	class EnKnight : public EnemyBase
	{
	public:
		EnKnight();
		void Init();
		void InitTutorial(const CharaVec2& pos, Dir4Type dir);
		void Update() override;
		double OrderPriority() const override;

		bool SendDamageCollider(ItemAttackerAffair& attacker, const RectF& collider) override;
		const CharaPosition& Pos() const override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
