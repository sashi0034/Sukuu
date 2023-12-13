#pragma once

#include "Forward.h"
#include "Play/PlayCore.h"

namespace Ending
{
	class EndingScene : public ActorBase
	{
	public:
		EndingScene();
		void Init(const Play::MeasuredSecondsArray& measured);
		void Update() override;

		bool IsFinished() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
