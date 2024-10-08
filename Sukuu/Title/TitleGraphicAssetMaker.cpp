﻿#include "stdafx.h"
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
			hud.ForceLogoData({.position = Scene::Center().movedBy(0, -172), .scale = 2.4});

			bg.SetPlayerPosition({1, 1});
		}
		else if (key == U"462x174")
		{
			pictureScale = 4;
			hud.ForceLogoData({.position = Scene::Center().movedBy(0, -120), .scale = 0});
			cam.focusPosition = Vec3{5, 1, 5};
			bg.SetPlayerPosition({3, 3});
		}
		else if (key == U"1232x706")
		{
			pictureScale = 1;
			hud.ForceLogoData({.position = Scene::Center().movedBy(0, -64), .scale = 2.2});
			cam.focusPosition = Vec3{5, 1, 5};
			bg.SetPlayerPosition({3, 3});
		}
		else if (key == U"748x896")
		{
			pictureScale = 1;
			hud.ForceLogoData({.position = Scene::Center().movedBy(0, 80), .scale = 1.5});
			cam.fov = 20_deg;
			cam.focusPosition = Vec3{0, 1, 0};
			bg.SetPlayerPosition({2, 2});
		}
		else if (key == U"600x900")
		{
			pictureScale = 1;
			hud.ForceLogoData({.position = Scene::Center().movedBy(0, 80), .scale = 1.2});
			cam.fov = 22_deg;
			cam.focusPosition = Vec3{0, 1, 0};
			bg.SetPlayerPosition({2, 2});
		}
		else if (key == U"920x430")
		{
			pictureScale = 1;
			hud.ForceLogoData({.position = Scene::Center().movedBy(0, -64), .scale = 2.2});
			cam.focusPosition = Vec3{5, 1, 5};
			bg.SetPlayerPosition({3, 3});
		}
		else if (key == U"3840x1240")
		{
			pictureScale = 0.5;
			hud.ForceLogoData({.position = Scene::Center().movedBy(0, -120), .scale = 0});
			cam.fov = 15_deg;
			cam.followHeight = 15;
			cam.followDistance = 35;
			cam.followAngle = 60_deg;
			cam.focusPosition = Vec3{0, 2.5, 0};
			bg.SetPlayerPosition({2, 0});
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
