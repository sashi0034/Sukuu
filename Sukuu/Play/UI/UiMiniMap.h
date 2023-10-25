#pragma once

#include "Play/Forward.h"

namespace Play
{
	class UiMiniMap : public ActorBase
	{
	public:
		UiMiniMap();
		void Init(const Size& mapSize);
		void Update() override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
