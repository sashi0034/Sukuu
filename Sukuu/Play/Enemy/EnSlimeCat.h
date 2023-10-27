#pragma once

#include "EnemyDefinition.h"

namespace Play
{
	class EnSlimeCat : public EnemyBase
	{
	public:
		EnSlimeCat();
		void Init();
		void Update() override;
		double OrderPriority() const override;

		bool SendDamageCollider(const RectF& collider) override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
