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
		void SetEnabled(bool enabled);

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
