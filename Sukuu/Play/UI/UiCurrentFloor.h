#pragma once

#include "Play/Forward.h"

namespace Play
{
	class UiCurrentFloor : public ActorBase
	{
	public:
		UiCurrentFloor();
		void Init(int floorIndex);
		void Update() override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
