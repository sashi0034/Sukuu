#pragma once

namespace Util
{
	[[nodiscard]]
	Array<Glyph> GetGlyphWithFallbacks(AssetNameView key, const String& text);
}
