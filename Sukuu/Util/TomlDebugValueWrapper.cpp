#include "stdafx.h"
#include "TomlDebugValueWrapper.h"

#include "Asserts.h"
#include "ErrorLogger.h"

namespace
{
	const FilePath debugPath = U"asset/debug.toml";
	const FilePath debugExamplePath = U"asset/debug.example.toml";

	void assertCoincidenceWithExample(const TOMLValue& users, const TOMLValue& example)
	{
		for (auto&& v : users.tableView())
		{
			if (not example.hasMember(v.name))
			{
				Util::ErrorLog(U"TOML parameter error: '{}' is missing in example."_fmt(v.name));
			}
			else if (v.value.getType() != example[v.name].getType())
			{
				Util::ErrorLog(U"TOML parameter error: '{}' type mismatched."_fmt(v.name));
			}
			else if (v.value.getType() == TOMLValueType::Table)
			{
				assertCoincidenceWithExample(v.value, example[v.name]);
			}
		}

		for (auto&& v : example.tableView())
		{
			if (not users.hasMember(v.name))
			{
				Util::ErrorLog(U"TOML parameter error: '{}' is reedundant in example."_fmt(v.name));
			}
		}
	}

	struct ImplState
	{
		DirectoryWatcher directoryWatcher{U"asset"};
		TOMLReader toml{debugPath};
		TOMLReader exampleToml{debugExamplePath};

		void Refresh()
		{
			for (auto [path, action] : directoryWatcher.retrieveChanges())
			{
				if (FileSystem::FileName(path) == U"debug.toml")
				{
					toml.open(debugPath);
					assertCoincidenceWithExample(toml, exampleToml);
				}
			}
		}
	};

	ImplState* s_instance{};

	void preInitialize()
	{
		FileSystem::Copy(debugExamplePath, debugPath, CopyOption::SkipExisting);
	}

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
			assertCoincidenceWithExample(m_state.toml, m_state.exampleToml);
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
	void InitTomlDebugValueAddon()
	{
		preInitialize();

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
