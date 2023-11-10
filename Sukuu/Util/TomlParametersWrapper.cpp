#include "stdafx.h"
#include "TomlParametersWrapper.h"

namespace Util
{
	namespace
	{
		struct ImplState
		{
			DirectoryWatcher directoryWatcher{U"asset"};
			TOMLReader toml{
#if _DEBUG
				U"asset/parameters.toml"
#else
				Resource(U"asset/parameters.toml")
#endif
			};
		};

		ImplState* s_instance;
	}

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
#if _DEBUG
  		for (auto [path, action] : p_impl->directoryWatcher.retrieveChanges())
		{
			if (FileSystem::FileName(path) == U"parameters.toml")
				p_impl->toml.open({U"asset/parameters.toml"});
		}
#endif
	}

	TOMLValue GetTomlParameters(const String& valuePath)
	{
		auto&& value = s_instance->toml[valuePath];;
#if _DEBUG
		if (value.isEmpty())
		{
			System::MessageBoxOK(U"TOML parameter error", U"'{}' is missing."_fmt(valuePath), MessageBoxStyle::Error);
		}
#endif
		return value;
	}
}
