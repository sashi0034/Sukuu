#include "stdafx.h"
#include "GlyphWithFallbacks.h"

#include "AssetKeys.h"

namespace Util
{
	// ラクラムシさんにご教示頂いたコードを元に作成
	// https://discord.com/channels/443310697397354506/998714158621147237/1272846496680771584
	// https://gist.github.com/Raclamusi/d5228f7386e8b9233634a79bbca1119b

	Array<Glyph> GetGlyphWithFallbacks(AssetNameView key, const String& text)
	{
		const auto fonts = AssetKeys::GetFontWithFallbacks(key);

		// .getGlyphClusters() でフォールバックを使用
		const auto glyphClusters = fonts[0].getGlyphClusters(text, UseFallback::Yes, Ligature::No);

		Array<Glyph> result{};
		for (const auto& [i, glyphCluster] : Indexed(glyphClusters))
		{
			const auto font = fonts[glyphCluster.fontIndex];

			// Glyph を取得
			Glyph glyph = font.getGlyph(
				text.substrView(
					glyphCluster.pos,
					(i == (glyphClusters.size() - 1))
						? String::npos
						: (glyphClusters[i + 1].pos - glyphCluster.pos)));

			// メインのフォントに合わせてスケーリング
			if (fonts[0].height() != font.height())
			{
				const double scale = (1.0 * fonts[0].height() / font.height());
				glyph.xAdvance *= scale;
				glyph.texture = glyph.texture.scaled(scale);
			}

			result.push_back(glyph);
		}

		return result;
	}
}
