#include "stdafx.h"

#include "AssetKeys.h"
#include "Assets.generated.h"
#include "Constants.h"
#include "Sukuu/AssetReloader.h"
#include "Gm/GameCursor.h"
#include "Gm/GamepadObserver.h"
#include "Sukuu/GamesSupervisor.h"
#include "Util/ActorContainer.h"
#include "Util/ErrorLogger.h"
#include "Util/TomlDebugValueWrapper.h"
#include "Util/TomlParametersWrapper.h"
#include "Util/Utilities.h"

using namespace Util;
using namespace Sukuu;

void Main()
{
	Scene::SetBackground(Constants::HardDarkblue);

	Window::SetTitle(Constants::GameIdentityTitleName);
	Window::SetStyle(WindowStyle::Sizable);
	Scene::SetResizeMode(ResizeMode::Keep);
	Scene::Resize(1920, 1080);
	Window::Resize(Size{1280, 720});
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);
#if not _DEBUG
	Window::SetFullscreen(true);
#endif
	System::Update();

#if _DEBUG
	for (auto&& path : AssetImages::GetKeys()) TextureAsset::Register(path, path);
	for (auto&& path : AssetSes::GetKeys()) AudioAsset::Register(path, path);
	for (auto&& path : AssetBgms::GetKeys()) AudioAsset::Register(path, Audio::Stream, path);
#else
	for (auto&& path : AssetImages::GetKeys()) TextureAsset::Register(path, Resource(path));
	for (auto&& path : AssetSes::GetKeys()) AudioAsset::Register(path, Resource(path));
	for (auto&& path : AssetBgms::GetKeys()) AudioAsset::Register(path, Audio::Stream, Resource(path));
#endif

	AssetKeys::RegisterAll();

	InitErrorLogger();
	InitTomlParametersAddon();

	// アドオン初期化
#if _DEBUG
	InitAssetReloader();
	InitTomlDebugParamAddon();
#endif

	// アクター初期化
	ActorContainer actorRoot{};
	actorRoot.Birth(Gm::GamepadObserver());
	actorRoot.Birth(GamesSupervisor());

	while (System::Update())
	{
		RefreshDeltaTime();

		actorRoot.Update();

		if (Gm::IsUsingGamepad()) Gm::RequestHideGameCursor();
		Gm::UpdateGameCursor();
	}
}

//
// - Debug ビルド: プログラムの最適化を減らす代わりに、エラーやクラッシュ時に詳細な情報を得られます。
//
// - Release ビルド: 最大限の最適化でビルドします。
//
// - [デバッグ] メニュー → [デバッグの開始] でプログラムを実行すると、[出力] ウィンドウに詳細なログが表示され、エラーの原因を探せます。
//
// - Visual Studio を更新した直後は、プログラムのリビルド（[ビルド]メニュー → [ソリューションのリビルド]）が必要な場合があります。
//
