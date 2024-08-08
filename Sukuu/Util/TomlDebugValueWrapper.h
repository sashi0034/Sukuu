#pragma once

namespace Util
{
#if _DEBUG
	void InitTomlDebugValueAddon();

	TOMLValue GetTomlDebugValue(const String& valuePath);

	template <typename T>
	inline T GetTomlDebugValueOf(const String& valuePath)
	{
		return GetTomlDebugValue(String(U"debug." + valuePath)).get<T>();;
	}
#endif
}
