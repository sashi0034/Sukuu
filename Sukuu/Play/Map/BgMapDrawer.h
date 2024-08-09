#pragma once
#include "Play/PlayCore.h"

namespace Play
{
	void DrawBgMapTileAt(const MapGrid& map, int x, int y);

	struct BgCustomDrawer
	{
		std::function<void(Rect)> backDrawer;
		std::function<void()> frontDrawer;
	};

	class BgMapDrawer
	{
	public:
		BgMapDrawer();
		void UpdateDraw();
		void PostDraw();
		void SetBgShader(const std::function<ScopedCustomShader2D(double t)>& shader);

		void SetCustomDrawer(const BgCustomDrawer& drawer);

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
