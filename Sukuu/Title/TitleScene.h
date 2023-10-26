#pragma once

#include "Forward.h"

namespace Title
{
	class TitleScene : public ActorBase
	{
	public:
		TitleScene();
		void Init();
		void Update() override;

		bool IsConcluded() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
