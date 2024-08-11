#pragma once
#include "LoungeMap.h"

namespace Lounge
{
	class LoungeBgDrawer
	{
	public:
		LoungeBgDrawer();

		void SetContinueFromMiddle(int continueFromMiddle);
		void DrawBack(const LoungeMapData& data, const Rect& region);
		void DrawFront(const LoungeMapData& data);

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
