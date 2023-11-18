#pragma once

#include "Play/Forward.h"

namespace Play
{
	class CaveSnowfall : public ActorBase
	{
	public:
		CaveSnowfall();
		void Update() override;
		double OrderPriority() const override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
