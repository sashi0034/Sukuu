#include "stdafx.h"

#include "AssetKeys.h"
#include "Assets.generated.h"
#include "Sukuu/AssetReloader.h"
#include "Sukuu/GamesSupervisor.h"
#include "Util/ActorContainer.h"
#include "Util/TomlParametersWrapper.h"

using namespace Util;
using namespace Sukuu;

void Main()
{
	Scene::SetBackground(ColorF{0.3, 0.3, 0.3});

	Window::SetStyle(WindowStyle::Sizable);
	Scene::SetResizeMode(ResizeMode::Keep);
	Scene::Resize(1920, 1080);
	Window::Resize(Size{1280, 720});
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	for (auto&& path : AssetImages::GetKeys()) TextureAsset::Register(path, path);
	for (auto&& path : AssetSes::GetKeys()) AudioAsset::Register(path, path);
	for (auto&& path : AssetBgms::GetKeys()) AudioAsset::Register(path, path);
	AssetKeys::RegisterAll();

	ActorContainer actorRoot{};
#if _DEBUG
	actorRoot.Birth(AssetReloader());
#endif
	actorRoot.Birth(TomlParametersWrapper());
	actorRoot.Birth(GamesSupervisor());

	while (System::Update())
	{
		actorRoot.Update();
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
