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

	ImplState* s_instance{};

	class TomlParametersWrapperAddon : public IAddon
	{
	private:
		ImplState m_state{};

	public:
		~TomlParametersWrapperAddon() override
		{
			if (s_instance == &m_state) s_instance = nullptr;
		}

		bool init() override
		{
			s_instance = &m_state;
			return true;
		}

		bool update() override
		{
			m_state.Refresh();
			return true;
		}
	};
}

namespace Util
{
	void InitTomlParametersAddon()
	{
		Addon::Register<TomlParametersWrapperAddon>(U"TomlParametersWrapperAddon");
	}

	TOMLValue GetTomlParameters(const String& valuePath)
	{
		auto&& value = s_instance->toml[valuePath];;
#if _DEBUG
		if (value.isEmpty())
		{
			Console.writeln(U"TOML parameter error", U"'{}' is missing."_fmt(valuePath));
		}
#endif
		return value;
	}

	void RefreshTomlParameters()
	{
		s_instance->Refresh();
	}
}
