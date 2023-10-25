#pragma once

#include "Play/Forward.h"

namespace Play
{
	class CaveVision
	{
	public:
		CaveVision();
		void UpdateMask(const Vec2& pos);
		void UpdateScreen();

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
