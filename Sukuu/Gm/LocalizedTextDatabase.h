#pragma once

namespace Gm
{
	void InitLocalizedTextDatabaseAddon();

	[[nodiscard]]
	const String& LocalizedText(StringView key);
}
