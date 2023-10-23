#pragma once
#include "Util/ActorBase.h"

namespace Sukuu
{
	class GamesSupervisor : public Util::ActorBase
	{
	public:
		GamesSupervisor();
		void Update() override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
