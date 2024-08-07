#include "stdafx.h"
#include "TitleGraphicAssetMaker.h"

namespace
{
	bool s_captureRequested{};
}

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

		struct
		{
			double fov = 12_deg;
			Vec3 focusPosition = Vec3{3.5, 1, 3.5};
			double followAngle = 45.0_deg;
			double followDistance = 40.0;
			double followHeight = 40.0;
		} cam{};

		// 画像サイズごとに追加の設定を行う
		if (key == U"920x430")
		{
			pictureScale = 2;
			hud.ForceLogoData({.position = Scene::Center().movedBy(0, -196), .scale = 3});

			bg.SetPlayerPosition({1, 1});
		}

		// Set camera
		const SimpleFollowCamera3D camera{
			Scene::Size(), cam.fov, cam.focusPosition, cam.followAngle, cam.followDistance, cam.followHeight
		};
		bg.ForceFixedCamera(camera);

		const auto pictureRect = RectF(Arg::center = pictureCenter, Size(pictureSizeX, pictureSizeY))
			.scaled(pictureScale);

		(void)pictureRect
		      .stretched(1)
		      .drawFrame(1, Palette::White);

		// S キーで画像保存
		if (KeyS.down())
		{
			ScreenCapture::RequestCurrentFrame();
			s_captureRequested = true;
		}
		else if (s_captureRequested)
		{
			s_captureRequested = false;

			// 画像保存
			const auto frame = ScreenCapture::GetFrame();
			const Image clipped = frame.clipped(pictureRect.asRect());
			const String filename = U"Screenshot/graphic_asset_{}.png"_fmt(key);
			(void)clipped
			      .scaled(pictureSizeX, pictureSizeY, InterpolationAlgorithm::Lanczos)
			      .save(filename);

			Console.writeln(U"Saved picture: {}"_fmt(filename));
		}
	}
}
