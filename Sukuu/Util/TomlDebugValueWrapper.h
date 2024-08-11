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

	template <typename T>
	inline Array<T> GetTomlDebugArrayOf(const String& valuePath)
	{
		Array<T> a{};
		for (const auto& v : GetTomlDebugValue(String(U"debug." + valuePath)).arrayView())
		{
			a.push_back(v.get<T>());
		}
		return a;;
	}
#endif
}
