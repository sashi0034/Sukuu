#include "stdafx.h"
#include "LocalizedTextDatabase.h"

#include "GameConfig.h"
#include "Util/Asserts.h"
#include "Util/ErrorLogger.h"

namespace
{
	using TranslatedText = String;

	class LocalizedTextDatabaseAddon;
	LocalizedTextDatabaseAddon* s_instance{};

	class LocalizedTextDatabaseAddon : public IAddon
	{
		HashTable<String, TranslatedText> m_caches{};
		Gm::GameLanguage m_cachedLanguage{};

		DirectoryWatcher m_directoryWatcher{U"asset"};
		INI m_ini{
#if _DEBUG
			U"asset/localize.ini"
#else
			Resource(U"asset/localize.ini")
#endif
		};

		String fromToml(const String& key, Gm::GameLanguage language) const
		{
#if _DEBUG
			if (not m_ini.hasSection(key))
			{
				Util::ErrorLog(U"\"{}\" does not exist in localize.ini"_fmt(key));
			}

			// 全ての言語に対応した状態であるかを一応確認
			for (int i = 0; i < Gm::GameLanguagesCount; ++i)
			{
				if (m_ini[key + U"." + Gm::LanguageCodes[i]].isEmpty())
				{
					Util::ErrorLog(U"\"{}\" does not have a translation for {}"_fmt(key, Gm::LanguageCodes[i]));
				}
			}
#endif

			if (m_ini.hasValue(key, Gm::LanguageCodes[static_cast<int>(language)]))
			{
				return m_ini.getValue(key, Gm::LanguageCodes[static_cast<int>(language)]);
			}

			return U"$" + key;
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

		const String& GetLocalizedText(const String& key, Gm::GameLanguage currentLanguage)
		{
			if (m_cachedLanguage != currentLanguage)
			{
				// 言語が切り替わった場合はキャッシュをクリア
				m_caches.clear();
				m_cachedLanguage = currentLanguage;
			}

			if (not m_caches.contains(key))
			{
				// キャッシュからデータを読み込む
				m_caches[key] = fromToml(key, currentLanguage);
			}

			return m_caches[key];
		}

		bool update() override
		{
#if _DEBUG
			for (auto [path, action] : m_directoryWatcher.retrieveChanges())
			{
				if (FileSystem::FileName(path) == U"localize.ini")
				{
					m_ini.clear();
					m_ini.load({U"asset/localize.ini"});

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

	const String& LocalizedText(StringView key)
	{
		if (not Util::AssertStrongly(s_instance != nullptr)) return {};

		const auto currentLanguage = GameConfig::Instance().language;
		return s_instance->GetLocalizedText(key.data(), currentLanguage);
	}
}
