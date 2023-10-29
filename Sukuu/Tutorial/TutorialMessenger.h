#pragma once

#include "Forward.h"

namespace Tutorial
{
	class TutorialMessenger : public ActorBase
	{
	public:
		TutorialMessenger();
		void Update() override;
		double OrderPriority() const override;

		void ShowMessage(const String& message, double duration);
		void ShowMessageForever(const String& message);
		void HideMessage();

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
