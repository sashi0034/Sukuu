#pragma once
#include "Play/PlayScene.h"

namespace Play
{
	void DrawBgMapTileAt(const MapGrid& map, int x, int y);

	class BgMapDrawer
	{
	public:
		BgMapDrawer();
		void UpdateDraw(const PlayScene& scene);
		void PostDraw();

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
