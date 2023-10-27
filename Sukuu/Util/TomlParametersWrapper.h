#pragma once
#include <source_location>

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
	inline T GetTomlParameter(const String& valuePath)
	{
		return GetTomlParameters(valuePath).get<T>();

		// TODO: リリースビルド用の処理を作成
	}
}
