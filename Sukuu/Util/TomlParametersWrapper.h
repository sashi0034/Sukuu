#pragma once
#include "ActorBase.h"

namespace Util
{
	class TomlParametersWrapper : public ActorBase
	{
	public:
		TomlParametersWrapper();
		void Update() override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};

	TOMLValue GetTomlParameters(const String& valuePath);

	template <typename T>
	T GetTomlParameter(const String& valuePath)
	{
#if _DEBUG
		return GetTomlParameters(valuePath).get<T>();
#else
		static bool initialized = false;
		static T value;
		if (not initialized)
		{
			value = GetTomlParameters(valuePath).get<T>();
			initialized = true;
		}
		return value;
#endif
	}
}
