#pragma once
#include "ActorContainer.h"
#include "CoroActor.h"
#include "Utilities.h"

namespace Util
{
	inline CoroActor StartCoro(ActorView parent, const CoroTaskFunc& coro)
	{
		return parent.AsParent().Birth(CoroActor(coro));
	}

	class YieldExtended : Uncopyable
	{
	public:
		YieldExtended(CoroTaskYield& y) : yield(y) { return; }

		void WaitForTime(double seconds, const std::function<double()>& dt = GetDeltaTime);
		void WaitForTrue(const std::function<bool()>& discriminant);
		void WaitForTrueVal(const bool& discriminant);
		void WaitForFalse(const std::function<bool()>& discriminant);
		void WaitForFalseVal(const bool& discriminant);
		void WaitForExpire(const ActorWeak& coro);
		void operator()() { yield(); }
		void operator()(int frame);

	private:
		CoroTaskYield& yield;
	};;
}
