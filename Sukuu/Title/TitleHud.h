#pragma once

#include "Forward.h"
#include "Sukuu/GameSavedata.h"

namespace Title
{
	class TitleHud : public ActorBase
	{
	public:
		TitleHud();
		void Init(const Sukuu::GameSavedata& savedata);
		void Update() override;
		double OrderPriority() const override;

		void SetShowPrompt(bool show);
		bool IsButtonHovered() const;
		bool IsConcludedRetryTutorial() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
