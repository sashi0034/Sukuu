#pragma once
#include "ActorBase.h"

namespace Util
{
	class EffectWrapper : public ActorBase
	{
	public:
		EffectWrapper();
		EffectWrapper(double orderPriority);
		void Update() override;
		double OrderPriority() const override;

		void add(std::unique_ptr<IEffect> effect);
		void add(const ActorBase& effect);

		Effect& GetEffect();
		const Effect& GetEffect() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
