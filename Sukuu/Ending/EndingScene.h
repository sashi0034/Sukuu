#pragma once

#include "Forward.h"

namespace Ending
{
	class EndingScene : public ActorBase
	{
	public:
		EndingScene();
		void Init();
		void Update() override;

		bool IsFinished() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
