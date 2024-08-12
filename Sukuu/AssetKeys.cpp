#include "AssetKeys.h"

#include "Util/ErrorLogger.h"

namespace
{
	constexpr std::array fallbackFontPaths{
		U"asset/font/NotoSans/NotoSansMonoCJKsc-Bold.otf"_sv,
		U"asset/font/NotoSans/NotoSansMonoCJKtc-Bold.otf"_sv,
		U"asset/font/NotoSans/NotoSansMonoCJKkr-Bold.otf"_sv,
	};

	String getFallbackKey(AssetNameView target, AssetNameView fallback)
	{
		return String(target.data()) + U" | " + String(fallback.data());
	}

	void registerFallbackTo(AssetNameView target)
	{
		auto&& targetFont = FontAsset(target);

		// 絵文字のフォールバックを追加
		if (not FontAsset::IsRegistered(U"ColorEmoji"))
		{
			FontAsset::Register(U"ColorEmoji", 32, Typeface::ColorEmoji);
		}
		auto&& emojiFont = FontAsset(U"ColorEmoji");
		if (not targetFont.addFallback(emojiFont))
		{
			Util::ErrorLog(U"Failed to add fallback font: {} <- ColorEmoji"_fmt(target));
		}

		// 外国語のフォールバックを追加
		for (int i = 0; i < fallbackFontPaths.size(); ++i)
		{
			const auto& path = fallbackFontPaths[i];

			// フォールバック用のフォントを登録
			const bool registered = FontAsset::Register(
				getFallbackKey(target, path), targetFont.method(), targetFont.fontSize(), path, targetFont.style());
			if (not registered) Util::ErrorLog(U"Failed to register fallback font: {} <- {}"_fmt(target, path));

			// ターゲットにフォールバックを追加
			const auto fb = FontAsset(getFallbackKey(target, path));
			if (fb.isEmpty()) Util::ErrorLog(U"Failed to load fallback font: {} <- {}"_fmt(target, path));

			if (not targetFont.addFallback(fb))
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
		PixelShaderAsset::Register(PsCaveVision, HLSL(PsCaveVision + U".hlsl"));
		PixelShaderAsset::Register(PsMultiTextureMask, HLSL(PsMultiTextureMask + U".hlsl"));
		PixelShaderAsset::Register(PsRasterScroll, HLSL(PsRasterScroll + U".hlsl"));
		PixelShaderAsset::Register(PsRgbToBgr, HLSL(PsRgbToBgr + U".hlsl"));
		PixelShaderAsset::Register(PsGrayscale, HLSL(PsGrayscale + U".hlsl"));
		PixelShaderAsset::Register(PsGradientBlur, HLSL(PsGradientBlur + U".hlsl"));
		VertexShaderAsset::Register(VsCaveVision, HLSL(VsCaveVision + U".hlsl"));
	}

	void RegisterAll()
	{
		// フォント登録
		FontAsset::Register(RocknRoll_24_Bitmap, 24, U"asset/font/RocknRoll/RocknRollOne-Regular.ttf");
		registerFallbackTo(RocknRoll_24_Bitmap);

		FontAsset::Register(RocknRoll_72_Bitmap, 72, U"asset/font/RocknRoll/RocknRollOne-Regular.ttf");
		registerFallbackTo(RocknRoll_72_Bitmap);

		FontAsset::Register(RocknRoll_Sdf, FontMethod::SDF, 48, U"asset/font/RocknRoll/RocknRollOne-Regular.ttf");
		registerFallbackTo(RocknRoll_Sdf);

		FontAsset::Register(
			RocknRoll_Sdf_Bold, FontMethod::SDF, 48, U"asset/font/RocknRoll/RocknRollOne-Regular.ttf", FontStyle::Bold);
		registerFallbackTo(RocknRoll_Sdf_Bold);

		// その他の登録
		TextureAsset::Register(gamepad, gamepad, TextureDesc::Mipped);

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
}
