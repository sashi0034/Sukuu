#pragma once

#include "Forward.h"

namespace Ending
{
	class EndingBackground : public ActorBase
	{
	public:
		EndingBackground();
		void Init();
		void Update();

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
