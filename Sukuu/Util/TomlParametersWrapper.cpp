#include "stdafx.h"
#include "TomlParametersWrapper.h"

namespace Util
{
	namespace
	{
		struct ImplState
		{
			DirectoryWatcher directoryWatcher{U"asset"};
			TOMLReader toml{U"asset/parameters.toml"};
		};
	}

	ImplState* s_instance;

	struct TomlParametersWrapper::Impl : ImplState
	{
	};

	TomlParametersWrapper::TomlParametersWrapper() :
		p_impl(std::make_shared<Impl>())
	{
		s_instance = p_impl.get();
	}

	void TomlParametersWrapper::Update()
	{
		for (auto [path, action] : p_impl->directoryWatcher.retrieveChanges())
		{
			if (FileSystem::FileName(path) == U"parameters.toml")
				p_impl->toml.open({U"asset/parameters.toml"});
		}
	}

	TOMLValue GetTomlParameters(const String& valuePath)
	{
		auto&& value = s_instance->toml[valuePath];;
#ifdef _DEBUG
		if (value.isEmpty())
		{
			System::MessageBoxOK(U"TOML parameter error", U"'{}' is missing."_fmt(valuePath), MessageBoxStyle::Error);
		}
#endif
		return value;
	}
}
