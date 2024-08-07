﻿#pragma once
#include "Forward.h"

namespace Lounge
{
	struct LoungeEnterArgs
	{
	};

	class LoungeScene : public ActorBase
	{
	public:
		LoungeScene();

		void Init(const LoungeEnterArgs& args);

		void Update() override;

		bool IsConcluded();

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
