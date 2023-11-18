#include "Utilities.h"

namespace Util
{
	namespace
	{
		double s_deltaTime{};
		double s_timeScale = 1.0;
	}

	void SetTimeScale(double scale)
	{
		s_timeScale = scale;
		RefreshDeltaTime();
	}

	double GetTimeScale()
	{
		return s_timeScale;
	}

	void RefreshDeltaTime()
	{
		s_deltaTime = s_timeScale * Scene::DeltaTime();
	}

	double GetDeltaTime()
	{
		return s_deltaTime;
	}
}
