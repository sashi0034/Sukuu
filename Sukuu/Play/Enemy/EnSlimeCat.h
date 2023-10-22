#pragma once
#include "IEnemy.h"

namespace Play
{
	class EnSlimeCat : public ActorBase, public IEnemy
	{
	public:
		EnSlimeCat();
		void Init();
		void Update() override;
		double OrderPriority() const override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
