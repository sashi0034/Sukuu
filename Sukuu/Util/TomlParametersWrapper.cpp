#include "stdafx.h"
#include "TomlParametersWrapper.h"

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

		void Refresh()
		{
			for (auto [path, action] : directoryWatcher.retrieveChanges())
			{
				if (FileSystem::FileName(path) == U"parameters.toml")
					toml.open({U"asset/parameters.toml"});
			}
		}
	};

	ImplState* s_instance;
}

namespace Util
{
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
		p_impl->Refresh();
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

	void RefreshTomlParameters()
	{
		s_instance->Refresh();
	}
}
