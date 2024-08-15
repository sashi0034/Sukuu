#pragma once

namespace Util
{
	[[nodiscard]]
	Array<Glyph> GetGlyphWithFallbacks(AssetNameView key, const String& text);

	[[nodiscard]]
	Array<OutlineGlyph> RenderOutlineGlyphWithFallbacks(AssetNameView key, const String& text);
}
