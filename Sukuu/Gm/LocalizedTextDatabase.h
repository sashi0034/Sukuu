#pragma once

namespace Gm
{
	void InitLocalizedTextDatabaseAddon();

	[[nodiscard]]
	StringView LocalizedText(StringView key);
}
