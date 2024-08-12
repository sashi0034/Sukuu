#pragma once

namespace Gm
{
	void InitLocalizedTextDatabaseAddon();

	[[nodiscard]]
	const String& LocalizedText(StringView key);

	inline const String& operator ""_localize(const char32_t* s, const size_t length) noexcept
	{
		return LocalizedText(StringView(s, length));
	}
}
