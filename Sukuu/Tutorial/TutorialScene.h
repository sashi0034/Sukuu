#pragma once

#include "Forward.h"

namespace Tutorial
{
	class TutorialScene : public ActorBase
	{
	public:
		TutorialScene();
		void Init();
		bool IsFinished() const;
		void Update() override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
