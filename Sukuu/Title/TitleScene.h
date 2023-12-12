#pragma once

#include "Forward.h"
#include "Gm/GameSavedata.h"

namespace Title
{
	class TitleScene : public ActorBase
	{
	public:
		TitleScene();
		void Init(const Gm::GameSavedata& savedata);
		void Update() override;
		void PerformReincarnate();

		bool IsConcludedPlay() const;
		bool IsConcludedRetryTutorial() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
