#include "stdafx.h"
#include "AchievementIconMaker.h"

namespace
{
	void renderNormal(Image& image, const Font& font, const String& text)
	{
		image.fill(ColorF{0.1, 0.1, 0.1});
		Circle{128, 128, 120}
			.paint(image, ColorF{0.14, 0.56, 0.82})
			.paintFrame(image, 0, 2, ColorF{1, 1, 1});
		font(text).paintAt(image, Rect(image.size()).center(), ColorF{1, 1, 1});
	}

	void preview()
	{
		Font font{FontMethod::MSDF, 128, Typeface::Mplus_Black, FontStyle::Default};

		DynamicTexture texture{};

		while (System::Update())
		{
			// -----------------------------
			Image image(256, 256);
			renderNormal(image, font, U"10");
			image.grayscale();
			for (auto& pixel : image)
			{
				pixel.r *= 0.5;
				pixel.g *= 0.5;
				pixel.b *= 0.5;
			}
			// -----------------------------

			texture.fill(image);
			texture.drawAt(Scene::Center());
		}
	}
}

void MakeAchievementIcon()
{
	// preview();

	Font font{FontMethod::MSDF, 128, Typeface::Mplus_Black, FontStyle::Default};
	Image image(256, 256);

	for (int i = 10; i <= 50; i += 10)
	{
		renderNormal(image, font, Format(i));
		image.saveJPEG(U"s" + Format(i) + U"_s.jpg");
	}

	for (int i = 10; i <= 50; i += 10)
	{
		renderNormal(image, font, Format(i));
		for (auto& pixel : image)
		{
			pixel.r *= 0.5;
			pixel.g *= 0.5;
			pixel.b *= 0.5;
		}
		image.grayscale().saveJPEG(U"s" + Format(i) + U"_g.jeg");
	}

	for (int i = 10; i <= 50; i += 10)
	{
		renderNormal(image, font, Format(i));
		image.negated().saveJPEG(U"u" + Format(i) + U"_s.jeg");
	}

	for (int i = 10; i <= 50; i += 10)
	{
		renderNormal(image, font, Format(i));
		image.negate();
		for (auto& pixel : image)
		{
			pixel.r *= 0.5;
			pixel.g *= 0.5;
			pixel.b *= 0.5;
		}
		image.grayscale().saveJPEG(U"u" + Format(i) + U"_g.jpg");
	}
}
