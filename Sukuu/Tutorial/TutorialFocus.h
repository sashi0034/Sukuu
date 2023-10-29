#pragma once

#include "Forward.h"

namespace Tutorial
{
	class TutorialFocus : public ActorBase
	{
	public:
		TutorialFocus();
		void Update() override;
		double OrderPriority() const override;

		void Show(const Vec2& pos);
		void Hide();

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
