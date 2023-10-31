#pragma once

#include "EnemyDefinition.h"

namespace Play
{
	class EnCrab : public EnemyBase
	{
	public:
		EnCrab();
		void Init();
		void Update() override;
		double OrderPriority() const override;

		bool SendDamageCollider(const ItemAttackerAffair& attacker, const RectF& collider) override;
		const CharaPosition& Pos() const override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
