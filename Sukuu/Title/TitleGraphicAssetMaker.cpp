#include "stdafx.h"
#include "TitleGraphicAssetMaker.h"

namespace Title
{
	void ProcessTitleGraphicAsset(const TitleGraphicAssetArgs& args)
	{
		const auto key = args.key;
		const int keyXIndex = key.indexOf(U'x');
		const int pictureSizeX = Parse<int>(key.substr(0, keyXIndex));
		const int pictureSizeY = Parse<int>(key.substr(keyXIndex + 1));

		Vec2 pictureCenter = Scene::Center();

		double pictureScale = 1;

		auto& hud = args.hud.get();
		auto& bg = args.bg.get();

		if (key == U"460x215")
		{
			pictureScale = 4;
			hud.ForceLogoData({.position = Scene::Center().movedBy(-120, -120), .scale = 3});
			bg.ForceCameraModifier([](SimpleFollowCamera3D& camera)
			{
				camera.jumpToTarget({0, 0, 0}, 45);
			});
		}

		const auto pictureRect = RectF(Arg::center = pictureCenter, Size(pictureSizeX, pictureSizeY))
			.scaled(pictureScale);

		(void)pictureRect
		      .stretched(1)
		      .drawFrame(1, Palette::White);
	}
}
