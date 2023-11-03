#include "stdafx.h"
#include "ItemObtainEffect.h"

#include "Assets.generated.h"

namespace
{
	struct ItemObtainEffect : IEffect
	{
		Vec2 m_center{};
		Array<double> m_particleDirs;

		explicit ItemObtainEffect(const Vec2& center) : m_center(center)
		{
			for (int i = 0; i < 6; ++i)
			{
				m_particleDirs.push_back(Random() * 2 * Math::Pi);
			}
		}

		bool update(double timeSec) override
		{
			drawCircleFrame(timeSec, 0.0, 0.5, 32, ColorF(1.0, 0.9, 0.6));
			drawCircleFrame(timeSec, 0.2, 0.5, 64, ColorF(1.0, 0.9, 0.6));
			drawRadiation(timeSec, 0.1, 0.4, 64, 5, 0.0, ColorF(1.0, 0.9, 0.6));
			drawRadiation(timeSec, 0.0, 0.5, 64, 5, 0.5, ColorF(1.0, 0.9, 0.6));

			for (int i = 0; i < m_particleDirs.size(); ++i)
			{
				drawParticle(timeSec, i * 0.05, i * 0.05 + 0.2, m_particleDirs[i], ColorF{1.0, 0.9, 0.1});
			}

			return timeSec < 0.5;
		}

		void drawCircleFrame(
			double timeSec, double startSec, double endSec,
			double r, const ColorF& color) const
		{
			if (InRange(timeSec, startSec, endSec))
			{
				const double alpha = 1.0 - EaseInQuart((timeSec - startSec) / (endSec - startSec));
				Circle(m_center, r * (timeSec - startSec) / endSec)
					.drawFrame(10,
					           ColorF{color, 0.0f}, ColorF{color, 0.3f * alpha});
			}
		}

		void drawRadiation(
			double timeSec, double startSec, double endSec,
			double r, int number, double offset, const ColorF& color) const
		{
			if (InRange(timeSec, startSec, endSec) == false) return;

			for (const auto i : step(number))
			{
				const double rate = (timeSec - startSec) / (endSec - startSec);
				const double alpha = 1.0 - EaseInQuart(rate);
				Circle(m_center, r * std::min(2 * rate, 1.0))
					.drawPie(
						Math::ToRadians((i + offset) * (360.0 / number)),
						Math::ToRadians(15),
						ColorF{color, 0.7f * alpha},
						ColorF{color, 0.0f});
			}
		}

		void drawParticle(
			double timeSec, double startSec, double endSec,
			double direction, const ColorF& color) const
		{
			if (InRange(timeSec, startSec, endSec) == false) return;

			const double rate = (timeSec - startSec) / (endSec - startSec);
			const double alpha = 1.0 - EaseInQuart(rate);

			const Vec2 offset = Circular(64 * rate, direction);

			(void)TextureAsset(AssetImages::particle)
			      .resized(8)
			      .drawAt(m_center.movedBy(offset), ColorF(color, 0.3 * alpha));
		}
	};
}

namespace Play
{
	std::unique_ptr<IEffect> MakeItemObtainEffect(const Vec2& center)
	{
		return std::make_unique<ItemObtainEffect>(center);
	}
}
