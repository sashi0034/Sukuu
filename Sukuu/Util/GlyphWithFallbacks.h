#pragma once

namespace Util
{
	struct GlyphWithFont
	{
		Glyph glyph;
		Font font;
	};

	[[nodiscard]]
	Array<GlyphWithFont> GetGlyphWithFallbacks(AssetNameView key, const String& text);

	[[nodiscard]]
	Array<OutlineGlyph> RenderOutlineGlyphWithFallbacks(AssetNameView key, const String& text);
}
