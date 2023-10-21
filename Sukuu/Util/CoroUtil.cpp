#include "stdafx.h"
#include "CoroUtil.h"

namespace Util
{
	CoroTask YieldExtended::WaitForTime(double seconds)
	{
		double totalTime = 0;
		while (totalTime < seconds)
		{
			totalTime += Scene::DeltaTime();
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
