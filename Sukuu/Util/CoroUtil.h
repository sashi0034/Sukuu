#pragma once
#include "ActorContainer.h"
#include "CoroActor.h"
#include "Utilities.h"

namespace Util
{
	inline CoroActor StartCoro(ActorBase& parent, const CoroTaskFunc& coro)
	{
		return parent.AsParent().Birth(CoroActor(coro));
	}

	class YieldExtended : Uncopyable
	{
	public:
		YieldExtended(CoroTaskYield& y) : yield(y) { return; }

		void WaitForTime(double seconds, const std::function<double()>& dt = GetDeltaTime);
		void WaitForTrue(const std::function<bool()>& discriminant);
		void WaitForFalse(const std::function<bool()>& discriminant);
		void WaitForDead(const ActorBase& coro);
		void operator()() { yield(); }

	private:
		CoroTaskYield& yield;
	};;
}
