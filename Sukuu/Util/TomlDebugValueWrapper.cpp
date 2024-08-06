#include "stdafx.h"
#include "TomlDebugValueWrapper.h"

#include "Asserts.h"
#include "ErrorLogger.h"

namespace
{
	struct ImplState
	{
		DirectoryWatcher directoryWatcher{U"asset"};
		TOMLReader toml{U"asset/debug.toml"};

		void Refresh()
		{
			for (auto [path, action] : directoryWatcher.retrieveChanges())
			{
				if (FileSystem::FileName(path) == U"debug.toml")
					toml.open({U"asset/debug.toml"});
			}
		}
	};

	ImplState* s_instance{};

	class TomlDebugValueWrapperAddon : public IAddon
	{
	private:
		ImplState m_state{};

	public:
		TomlDebugValueWrapperAddon()
		{
			if (not Util::AssertStrongly(s_instance == nullptr)) return;
			s_instance = &m_state;
		}

		~TomlDebugValueWrapperAddon() override
		{
			if (s_instance == &m_state) s_instance = nullptr;
		}

		bool init() override
		{
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
	void InitTomlDebugParamAddon()
	{
		Addon::Register<TomlDebugValueWrapperAddon>(U"TomlDebugValueWrapperAddon");
	}

	TOMLValue GetTomlDebugValue(const String& valuePath)
	{
		auto&& value = s_instance->toml[valuePath];;

		if (value.isEmpty())
		{
			ErrorLog(U"TOML parameter error: '{}' is missing."_fmt(valuePath));
		}

		return value;
	}
}
