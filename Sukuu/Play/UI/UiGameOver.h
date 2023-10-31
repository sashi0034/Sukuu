#pragma once

#include "Play/Forward.h"

namespace Play
{
	class UiGameOver : public ActorBase
	{
	public:
		UiGameOver();
		void Init(int floorIndex);
		void Update() override;
		double OrderPriority() const override;

		ActorWeak StartPerform();

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
