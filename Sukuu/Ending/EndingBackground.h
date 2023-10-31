#pragma once

#include "Forward.h"

namespace Ending
{
	class EndingBackground : public ActorBase
	{
	public:
		EndingBackground();
		void Init();
		void Update() override;

		Vec2 PlainSize() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
