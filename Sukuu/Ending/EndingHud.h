#pragma once

#include "Forward.h"
#include "Play/PlayScene.h"

namespace Ending
{
	class EndingHud : public ActorBase
	{
	public:
		EndingHud();
		void Init(const Play::MeasuredSecondsArray& measured);
		void Update() override;
		bool IsFinished() const;

		double OrderPriority() const override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
