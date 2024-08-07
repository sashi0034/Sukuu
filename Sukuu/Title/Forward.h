#pragma once

#include "Util/ActorBase.h"
#include "Util/TomlParametersWrapper.h"

namespace Title
{
	using namespace Util;

	using CameraModifierType = std::function<void(std::reference_wrapper<SimpleFollowCamera3D>)>;

	struct TitleLogoData
	{
		Vec2 position;
		double scale;
	};
}
