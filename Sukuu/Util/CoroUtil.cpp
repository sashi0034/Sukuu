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
		while (not discriminant())
		{
			yield();
		}
	}

	void YieldExtended::WaitForTrueVal(const bool& discriminant)
	{
		while (not discriminant)
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

	void YieldExtended::WaitForFalseVal(const bool& discriminant)
	{
		while (discriminant)
		{
			yield();
		}
	}

	CoroTask YieldExtended::WaitForDead(ActorView coro)
	{
		WaitForTrue([coro]() { return coro.IsDead(); });
	}

	void YieldExtended::WaitForExpire(const ActorWeak& coro)
	{
		WaitForTrue([coro]() { return coro.IsDead(); });
	}

	void YieldExtended::operator()(int frame)
	{
		for (const auto i : step(frame))
		{
			yield();
		}
	}
}
