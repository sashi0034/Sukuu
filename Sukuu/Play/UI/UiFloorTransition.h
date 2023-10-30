#pragma once

#include "Play/Forward.h"

namespace Play
{
	class UiFloorTransition : public ActorBase
	{
	public:
		UiFloorTransition();
		void Init();
		void Update() override;
		double OrderPriority() const override;

		void CloseRadial();

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
