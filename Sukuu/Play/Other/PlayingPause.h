#pragma once
#include "Play/Forward.h"

namespace Play
{
	class PlayingPause : public ActorBase
	{
	public:
		PlayingPause();
		void Init(bool enableRetire);
		bool IsPaused() const;
		void SetAllowed(bool e);
		void Update() override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
