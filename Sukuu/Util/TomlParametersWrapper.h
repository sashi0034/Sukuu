#pragma once
#include <source_location>

namespace Util
{
	void InitTomlParametersAddon();

	TOMLValue GetTomlParameters(const String& valuePath);

	template <typename T>
	inline T GetTomlParameter(const String& valuePath)
	{
#if _DEBUG
		return GetTomlParameters(valuePath).get<T>();;
#else
		static HashTable<String, T> s_hash{};
		auto&& found = s_hash.find(valuePath);
		if (found != s_hash.end()) return found->second;
		auto loaded = GetTomlParameters(valuePath).get<T>();
		s_hash[valuePath] = loaded;
		return loaded;
#endif
	}

	void RefreshTomlParameters();
}
