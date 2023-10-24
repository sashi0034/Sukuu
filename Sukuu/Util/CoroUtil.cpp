#include "stdafx.h"
#include "CoroUtil.h"

#include "Utilities.h"

namespace Util
{
	void YieldExtended::WaitForTime(double seconds, const std::function<double()>& dt)
	{
		double totalTime = 0;
		while (totalTime < seconds)
		{
			totalTime += dt();
			yield();
		}
	}

	CoroTask YieldExtended::WaitForTrue(const std::function<bool()>& discriminant)
	{
		while (!discriminant())
		{
			yield();
		}
	}

	CoroTask YieldExtended::WaitForFalse(const std::function<bool()>& discriminant)
	{
		while (discriminant())
		{
			yield();
		}
	}

	CoroTask YieldExtended::WaitForDead(const ActorBase& coro)
	{
		WaitForTrue([coro]() { return coro.IsDead(); });
	}
}
