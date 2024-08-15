#include "AssetKeys.h"

#include "Util/ErrorLogger.h"

#if _DEBUG
#define ASSET_PATH(path) path
#else
#define ASSET_PATH(path) Resource(path)
#endif

namespace
{
	HashTable<AssetName, Array<Font>> s_fontWithFallbacks{};

	constexpr std::array<StringView, 3> fallbackFontPaths{
		U"asset/font/NotoSans/NotoSansMonoCJKsc-Bold.otf"_sv,
		U"asset/font/NotoSans/NotoSansMonoCJKtc-Bold.otf"_sv,
		U"asset/font/NotoSans/NotoSansMonoCJKkr-Bold.otf"_sv,
	};

	bool addFallbackTo(AssetNameView target, const Font& fallbackFont)
	{
		auto&& targetFont = FontAsset(target);
		if (not targetFont.addFallback(fallbackFont)) return false;

		s_fontWithFallbacks[target].push_back(fallbackFont);
		return true;
	}

	String getFallbackKey(AssetNameView target, AssetNameView fallback)
	{
		return String(target.data()) + U" | " + String(fallback.data());
	}

	void registerFallbackTo(AssetNameView target)
	{
		auto&& targetFont = FontAsset(target);
		s_fontWithFallbacks[target] = {Font(targetFont)};

		// 絵文字のフォールバックを追加
		if (not FontAsset::IsRegistered(U"ColorEmoji"))
		{
			FontAsset::Register(U"ColorEmoji", 32, Typeface::ColorEmoji);
		}
		auto&& emojiFont = FontAsset(U"ColorEmoji");
		if (not addFallbackTo(target, emojiFont))
		{
			Util::ErrorLog(U"Failed to add fallback font: {} <- ColorEmoji"_fmt(target));
		}

		// 外国語のフォールバックを追加
		for (int i = 0; i < fallbackFontPaths.size(); ++i)
		{
			const auto& path = fallbackFontPaths[i];

			// フォールバック用のフォントを登録
			const bool registered = FontAsset::Register(
				getFallbackKey(target, path),
				targetFont.method(),
				targetFont.fontSize(),
				ASSET_PATH(path),
				targetFont.style());
			if (not registered) Util::ErrorLog(U"Failed to register fallback font: {} <- {}"_fmt(target, path));

			// ターゲットにフォールバックを追加
			const auto fb = FontAsset(getFallbackKey(target, path));
			if (fb.isEmpty()) Util::ErrorLog(U"Failed to load fallback font: {} <- {}"_fmt(target, path));

			if (not addFallbackTo(target, fb))
			{
				Util::ErrorLog(U"Failed to add fallback font: {} <- {}"_fmt(target, path));
			}
		}
	}
}

namespace AssetKeys
{
	void RegisterShader()
	{
		PixelShaderAsset::Register(PsCaveVision, HLSL(ASSET_PATH(PsCaveVision + U".hlsl")));
		PixelShaderAsset::Register(PsMultiTextureMask, HLSL(ASSET_PATH(PsMultiTextureMask + U".hlsl")));
		PixelShaderAsset::Register(PsRasterScroll, HLSL(ASSET_PATH(PsRasterScroll + U".hlsl")));
		PixelShaderAsset::Register(PsRgbToBgr, HLSL(ASSET_PATH(PsRgbToBgr + U".hlsl")));
		PixelShaderAsset::Register(PsGrayscale, HLSL(ASSET_PATH(PsGrayscale + U".hlsl")));
		PixelShaderAsset::Register(PsGradientBlur, HLSL(ASSET_PATH(PsGradientBlur + U".hlsl")));
		VertexShaderAsset::Register(VsSoftShape, HLSL(ASSET_PATH(VsSoftShape + U".hlsl")));
	}

	// 反省: 今後はシェーダーの Resource.rc も自動生成を行う

	void RegisterAll()
	{
		s_fontWithFallbacks.clear();

		// フォント登録
		FontAsset::Register(
			RocknRoll_24_Bitmap, 24, ASSET_PATH(U"asset/font/RocknRoll/RocknRollOne-Regular.ttf"));
		registerFallbackTo(RocknRoll_24_Bitmap);

		FontAsset::Register(
			RocknRoll_72_Bitmap, 72, ASSET_PATH(U"asset/font/RocknRoll/RocknRollOne-Regular.ttf"));
		registerFallbackTo(RocknRoll_72_Bitmap);

		FontAsset::Register(
			RocknRoll_Sdf, FontMethod::SDF, 48, ASSET_PATH(U"asset/font/RocknRoll/RocknRollOne-Regular.ttf"));
		registerFallbackTo(RocknRoll_Sdf);

		FontAsset::Register(
			RocknRoll_Sdf_Bold, FontMethod::SDF, 48, ASSET_PATH(U"asset/font/RocknRoll/RocknRollOne-Regular.ttf"),
			FontStyle::Bold);
		registerFallbackTo(RocknRoll_Sdf_Bold);

		// その他の登録
		TextureAsset::Register(gamepad, ASSET_PATH(gamepad), TextureDesc::Mipped);

		TextureAsset::Register(U"👉", U"👉"_emoji);
		TextureAsset::Register(U"💛", U"💛"_emoji);

		TextureAsset::Register(U"⏳", U"⏳"_emoji);
		TextureAsset::Register(U"😎", U"😎"_emoji);
		TextureAsset::Register(U"🤔", U"🤔"_emoji);
		TextureAsset::Register(U"😴", U"😴"_emoji);
		TextureAsset::Register(U"😵", U"😵"_emoji);
		TextureAsset::Register(U"🤗", U"🤗"_emoji);

		TextureAsset::Register(U"🪶", U"🪶"_emoji);
		TextureAsset::Register(U"⛑️", U"⛑️"_emoji);
		TextureAsset::Register(U"📌", U"📌"_emoji);
		TextureAsset::Register(U"💣", U"💣"_emoji);
		TextureAsset::Register(U"💡", U"💡"_emoji);
		TextureAsset::Register(U"🧲", U"🧲"_emoji);
		TextureAsset::Register(U"🔖", U"🔖"_emoji);
		TextureAsset::Register(U"🗂️", U"🗂️"_emoji);
		TextureAsset::Register(U"🪦", U"🪦"_emoji);
		TextureAsset::Register(U"🌞", U"🌞"_emoji);
		TextureAsset::Register(U"🧪", U"🧪"_emoji);
		TextureAsset::Register(U"🧂", U"🧂"_emoji);
		TextureAsset::Register(U"🚀", U"🚀"_emoji);

		RegisterShader();
	}

	Array<Font> GetFontWithFallbacks(AssetNameView name)
	{
		return s_fontWithFallbacks[name];
	}
}
