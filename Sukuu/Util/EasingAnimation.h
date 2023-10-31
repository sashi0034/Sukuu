#pragma once
#include "ActorBase.h"
#include "ActorContainer.h"

namespace Util
{
	namespace EaseOption
	{
		enum : uint64
		{
			EndStop = 1 << 0,
			IgnoreTimeScale = 1 << 1,
		};

		constexpr uint64 None = 0;

		constexpr uint64 Default =
			EndStop;
	};

	template <double easing(double), uint64 option, typename T>
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
			if constexpr (option & EaseOption::IgnoreTimeScale)
				m_state->time += Scene::DeltaTime();
			else
				m_state->time += GetDeltaTime();

			if (m_state->time >= m_state->duration)
			{
				if constexpr (option & EaseOption::EndStop) m_state->time = m_state->duration;
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

	template <double easing(double), uint64 option = EaseOption::Default, typename T>
	EasingAnimation<easing, option, T> AnimateEasing(ActorView parent, T* valuePtr, T endValue, double duration)
	{
		return parent.AsParent().Birth(EasingAnimation<easing, option, T>(valuePtr, endValue, duration));
	}

	constexpr double BoomerangParabola(double value)
	{
		return 1 - 4 * (value - 0.5) * (value - 0.5);
	}

	template <int wave = 1>
	constexpr double BoomerangSin(double value)
	{
		return Math::Sin(wave * Math::Pi * value);
	}
}
