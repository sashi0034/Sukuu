#pragma once
#include "Forward.h"

namespace Lounge
{
	struct LoungeEnterArgs
	{
		int reachedFloor;
	};

	class LoungeScene : public ActorBase
	{
	public:
		LoungeScene();

		void Init(const LoungeEnterArgs& args);

		void Update() override;

		bool IsConcluded();

		bool IsReturnToTitle() const;
		int NextFloor() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
