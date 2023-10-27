#include "stdafx.h"
#include "FragmentTextureEffect.h"

namespace Play
{
	struct FragmentTextureEffect : IEffect
	{
		Vec2 m_center;
		TextureRegion m_texture;
		Rect m_region;
		Array<Trail> m_trails{};
		Color m_trailColor{};
		int m_diffusion{};
		bool m_mirrored{};
		bool m_flipped{};

		FragmentTextureEffect(
			const Vec2& center, const TextureRegion& texture, const Color& color, int diffusion):
			m_center(center),
			m_texture(texture),
			m_trailColor(color),
			m_diffusion(diffusion)
		{
			m_trails.resize(4, Trail(0.3));
			auto&& size = m_texture.texture.size();
			int x1 = m_texture.uvRect.left * size.x;
			int y1 = m_texture.uvRect.top * size.y;
			int x2 = m_texture.uvRect.right * size.x;
			int y2 = m_texture.uvRect.bottom * size.y;
			if (x1 > x2)
			{
				m_mirrored = true;
				std::swap(x1, x2);;
			}
			if (y1 > y2)
			{
				m_flipped = true;
				std::swap(y1, y2);
			}
			m_region = Rect{x1, y1, x2 - x1, y2 - y1};
		}

		bool update(double timeSec) override
		{
			for (auto&& trail : m_trails)
			{
				trail.draw();
				trail.update();
			}

			const auto half = m_region.size / 2;
			drawFragment(timeSec, 0, 0.5, m_trails[0], Point{}, {-1, -1});
			drawFragment(timeSec, 0, 0.5, m_trails[1], half.y0().yx(), {-1, 1});
			drawFragment(timeSec, 0, 0.5, m_trails[2], half.x0(), {1, -1});
			drawFragment(timeSec, 0, 0.5, m_trails[3], half, {1, 1});

			return timeSec < 0.5 + m_trails[0].getLifeTime();
		}

		void drawFragment(
			double timeSec, double startSec, double endSec,
			Trail& trail, const Point& clip, const Vec2& dir) const
		{
			Point uvClip = clip;
			if (m_mirrored)
			{
				uvClip.x = m_region.size.x / 2 - clip.x;
			}
			if (m_flipped)
			{
				uvClip.y = m_region.size.y / 2 - clip.y;
			}
			if (InRange(timeSec, startSec, endSec) == false) return;
			const double rate = EaseInSine((timeSec - startSec) / (endSec - startSec));

			const auto offset1 = clip + dir * m_diffusion * rate;
			const auto offset2 = (1 - 4 * (0.5 - rate) * (0.5 - rate)) * Vec2{0, -Math::Abs(m_region.size.y) * 2.0};
			const auto scale = rate < 0.5 ? 1 : 1 - EaseInBack(2 * (rate - 0.5));
			const auto drawn = m_texture.texture(Rect{m_region.pos + uvClip, m_region.size / 2})
			                            .scaled(1, scale)
			                            .mirrored(m_mirrored)
			                            .flipped(m_flipped)
			                            .drawAt(m_center - m_region.size / 4 + offset1 + offset2);

			trail.add(drawn.center().moveBy(-dir * drawn.size / 2), m_trailColor, 10 * scale);
		}
	};

	std::unique_ptr<IEffect> EmitFragmentTextureEffect(
		const Vec2& center, const TextureRegion& texture, const Color& trailColor, int diffusion)
	{
		return std::make_unique<FragmentTextureEffect>(center, texture, trailColor, diffusion);
	}
}
