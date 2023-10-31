#pragma once

#include "Forward.h"

namespace Title
{
	class TitleLogo : public ActorBase
	{
	public:
		TitleLogo();
		void Update() override;
		double OrderPriority() const override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
