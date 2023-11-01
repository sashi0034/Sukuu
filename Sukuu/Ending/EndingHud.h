#pragma once

#include "Forward.h"

namespace Ending
{
	class EndingHud : public ActorBase
	{
	public:
		EndingHud();
		void Init();
		void Update();

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
