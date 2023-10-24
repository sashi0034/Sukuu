#include "Utilities.h"

namespace Util
{
	namespace
	{
		double s_timeScale = 1.0;
	}

	void SetTimeScale(double scale)
	{
		s_timeScale = scale;
	}

	double GetTimeScale()
	{
		return s_timeScale;
	}

	double GetDeltaTime()
	{
		return s_timeScale * Scene::DeltaTime();
	}
}
