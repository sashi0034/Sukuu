#pragma once

#include "Play/Forward.h"

namespace Play
{
	class UiItemContainer : public ActorBase
	{
	public:
		UiItemContainer();
		void Update() override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
};
