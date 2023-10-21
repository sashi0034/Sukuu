#pragma once
#include "Forward.h"

namespace Play
{
	class PlayScene : public ActorBase
	{
	public:
		PlayScene();
		void Update() override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
