#include "stdafx.h"
#include "LocalizedTextDatabase.h"

#include "GameConfig.h"
#include "Util/Asserts.h"
#include "Util/ErrorLogger.h"

namespace
{
	using TranslationTuple = std::array<String, Gm::GameLanguagesCount>;

	class LocalizedTextDatabaseAddon;
	LocalizedTextDatabaseAddon* s_instance{};

	class LocalizedTextDatabaseAddon : public IAddon
	{
		HashTable<String, TranslationTuple> m_caches{};

		DirectoryWatcher m_directoryWatcher{U"asset"};
		TOMLReader m_toml{
#if _DEBUG
			U"asset/localize.toml"
#else
			Resource(U"asset/localize.toml")
#endif
		};

		String fromToml(const String& key, Gm::GameLanguage language) const
		{
			auto&& node = m_toml[key];
			if (node.isEmpty())
			{
				Util::ErrorLog(U"\"{}\" does not exist in localize.toml"_fmt(key));
				return U"(undefined)";
			}

#if _DEBUG
			// 全ての言語に対応した状態であるかを一応確認
			for (int i = 0; i < Gm::GameLanguagesCount; ++i)
			{
				if (node[Gm::LanguageCodes[i]].isEmpty())
				{
					Util::ErrorLog(U"\"{}\" does not have a translation for {}"_fmt(key, Gm::LanguageCodes[i]));
				}
			}
#endif

			auto&& leaf = node[Gm::LanguageCodes[static_cast<int>(language)]];
			if (leaf.isEmpty()) return U"(unknown)";
			return leaf.getString();
		}

	public:
		LocalizedTextDatabaseAddon()
		{
			if (not Util::AssertStrongly(s_instance == nullptr)) return;
			s_instance = this;
		}

		~LocalizedTextDatabaseAddon() override
		{
			if (s_instance == this) s_instance = nullptr;
		}

		String GetLocalizedText(const String& key, Gm::GameLanguage currentLanguage)
		{
			if (not m_caches.contains(key))
			{
				TranslationTuple data{};
				data[static_cast<int>(currentLanguage)] = fromToml(key, currentLanguage);
				m_caches[key] = data;
			}

			return m_caches[key][static_cast<int>(currentLanguage)];
		}

		bool update() override
		{
#if _DEBUG
			for (auto [path, action] : m_directoryWatcher.retrieveChanges())
			{
				if (FileSystem::FileName(path) == U"localize.toml")
				{
					m_toml.open({U"asset/localize.toml"});
					m_caches.clear();
				}
			}
#endif

			return true;
		}
	};
}

namespace Gm
{
	void InitLocalizedTextDatabaseAddon()
	{
		Addon::Register<LocalizedTextDatabaseAddon>(U"LocalizedTextDatabaseAddon");
	}

	StringView LocalizedText(StringView key)
	{
		const auto currentLanguage = GameConfig::Instance().language;
		return s_instance->GetLocalizedText(key.data(), currentLanguage);
	}
}
