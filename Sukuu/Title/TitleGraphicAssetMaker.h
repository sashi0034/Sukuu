#pragma once

#include "TitleBackground.h"
#include "TitleHud.h"

namespace Title
{
	struct TitleGraphicAssetArgs
	{
		String key;
		std::reference_wrapper<TitleBackground> bg;
		std::reference_wrapper<TitleHud> hud;
	};

	/// @brief Steam のタイトルグラフィックアセットを作成するための処理
	/// これは毎フレーム実行する
	void ProcessTitleGraphicAsset(const TitleGraphicAssetArgs& args);
}
