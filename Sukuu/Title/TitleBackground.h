#pragma once

#include "Forward.h"

namespace Title
{
	class TitleBackground : public ActorBase
	{
	public:
		TitleBackground();
		void Init();
		void Update() override;
		void SetCameraTimescale(double ts);

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
