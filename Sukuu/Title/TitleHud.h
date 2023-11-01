#pragma once

#include "Forward.h"

namespace Title
{
	class TitleHud : public ActorBase
	{
	public:
		TitleHud();
		void Init();
		void Update() override;
		double OrderPriority() const override;

		void SetShowPrompt(bool show);
		bool IsCreditHovered() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
