#pragma once
#include "Play/PlayCore.h"

namespace Play
{
	void DrawBgMapTileAt(const MapGrid& map, int x, int y);

	class BgMapDrawer
	{
	public:
		BgMapDrawer();
		void UpdateDraw();
		void PostDraw();
		void SetBgShader(const std::function<ScopedCustomShader2D(double t)>& shader);

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
