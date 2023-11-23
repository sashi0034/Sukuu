#pragma once

#include "Play/Forward.h"

namespace Play
{
	class UiDashKeep : public ActorBase
	{
	public:
		UiDashKeep();
		void Init(bool isKeeping);
		void Update() override;
		bool IsKeeping() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
