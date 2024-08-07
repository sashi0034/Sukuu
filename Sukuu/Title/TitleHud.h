﻿#pragma once

#include "Forward.h"
#include "Gm/GameSavedata.h"

namespace Title
{
	class TitleHud : public ActorBase
	{
	public:
		TitleHud();
		void Init(const Gm::GameSavedata& savedata);
		void Update() override;
		double OrderPriority() const override;

		void SetShowPrompt(bool show);
		bool IsConcludedPlay() const;
		bool IsConcludedRetryTutorial() const;

		void ForceLogoData(const TitleLogoData& data);

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
