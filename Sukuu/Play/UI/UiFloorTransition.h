#pragma once

#include "Play/Forward.h"

namespace Play
{
	class UiFloorTransition : public ActorBase
	{
	public:
		UiFloorTransition();
		void Init();
		bool IsInitialized() const;
		void Update() override;
		double OrderPriority() const override;

		ActorWeak PerformOpen(int floorIndex);
		ActorWeak PerformClose();

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
