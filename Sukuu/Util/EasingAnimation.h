#pragma once
#include "ActorBase.h"
#include "ActorContainer.h"

namespace ExUiBasic
{
	template <double easing(double), typename T>
	class EasingAnimation : public ActorBase
	{
	public:
		explicit EasingAnimation(T* valuePtr, T endValue, double duration):
			m_state(std::make_shared<State>(State{
				.valuePtr = (valuePtr),
				.startValue = (*valuePtr),
				.endValue = (endValue),
				.time = 0,
				.duration = (duration)
			}))
		{
		}

		void Update() override
		{
			m_state->time += Scene::DeltaTime();
			if (m_state->time >= m_state->duration)
			{
				m_state->time = m_state->duration;
				Kill();
			}
			const double e = easing(m_state->time / m_state->duration);
			*(m_state->valuePtr) = m_state->startValue * (1 - e) + m_state->endValue * e;
		}

	private:
		struct State
		{
			T* valuePtr;
			T startValue;
			T endValue;
			double time;
			double duration;
		};

		std::shared_ptr<State> m_state{};
	};

	template <double easing(double), typename T>
	EasingAnimation<easing, T> AnimateEasing(ActorBase& parent, T* valuePtr, T endValue, double duration)
	{
		return parent.AsParent().Birth(EasingAnimation<easing, T>(valuePtr, endValue, duration));
	}

	constexpr double BoomerangParabola(double value)
	{
		return 1 - 4 * (value - 0.5) * (value - 0.5);
	}
}
