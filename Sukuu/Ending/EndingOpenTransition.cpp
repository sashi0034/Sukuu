#include "stdafx.h"
#include "EndingOpenTransition.h"

namespace
{
	using namespace Ending;

	constexpr int phaseCount = 3;

	struct EndingOpeningTransitionEffect : public IEffect
	{
		EndingOpenTransitionArgs m_args;
		double m_phaseDuration{};

		explicit EndingOpeningTransitionEffect(const EndingOpenTransitionArgs& args): m_args(args)
		{
			m_phaseDuration = m_args.basicDuration / phaseCount;
		}

		bool update(double timeSec) override
		{
			constexpr Size blockSize{Size::One() * 60};

			bool completed{true};

			const int maxX = Scene::Size().x / blockSize.x;
			const double columnDelta = m_args.basicDuration / maxX;
			for (int x = 0; x < maxX; ++x)
			{
				// 左の四角のほうが遅れて小さくなるようにする
				double columnSec = timeSec - ((maxX - 1 - x) * columnDelta);
				const int phase = static_cast<int>(Math::Clamp(columnSec / m_phaseDuration, 0.0, phaseCount));
				double scale = 1.0 - (static_cast<double>(phase) / phaseCount);

				if (phase < phaseCount) completed = false;

				for (int y = 0; y < Scene::Size().y / blockSize.y; ++y)
				{
					RectF blockRect{blockSize * Point{x, y}, blockSize};
					(void)blockRect.scaledAt(blockRect.center(), scale).draw(m_args.fg);
				}
			}

			return not completed;
		};
	};
}

namespace Ending
{
	std::unique_ptr<IEffect> CreateEndingOpenTransition(const EndingOpenTransitionArgs& args)
	{
		return std::make_unique<EndingOpeningTransitionEffect>(args);
	}
}
