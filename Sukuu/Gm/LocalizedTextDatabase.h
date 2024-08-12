#pragma once

namespace Gm
{
	void InitLocalizedTextDatabaseAddon();

	[[nodiscard]]
	const String& LocalizedText(StringView key);

	inline namespace Literals
	{
		inline const String& operator ""_localize(const char32_t* s, const size_t length)
		{
			return LocalizedText(StringView(s, length));
		}

		inline detail::FormatHelper operator ""_localizef(const char32* s, const size_t length)
		{
			return detail::FormatHelper{LocalizedText(StringView(s, length)).data()};
		}
	}
}

using namespace Gm::Literals;
