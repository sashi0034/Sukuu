﻿#include "stdafx.h"
#include "PlayerInternal.h"

#include "Play/PlayScene.h"
#include "Util/CoroUtil.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.player." + key);
	}
}

namespace Play
{
	bool CheckUseItemLightBulb(ActorView self, PlayerVisionState& vision)
	{
		// 視界クリア
		if (vision.mistRemoval) return false;
		vision.mistRemoval = true;
		StartCoro(self, [&vision](YieldExtended yield)
		{
			yield.WaitForTime(getToml<double>(U"light_bulb_duration"));
			vision.mistRemoval = false;
		});
		return true;
	}

	bool CheckUseItemExplorer(ActorView self)
	{
		// エネミーを一定時間全表示
		auto&& minimap = PlayScene::Instance().GetMiniMap();
		if (minimap.IsShowEnemies()) return false;
		minimap.SetShowEnemies(true);
		StartCoro(self, [](YieldExtended yield)
		{
			yield.WaitForTime(getToml<double>(U"explorer_enemies_duration"));
			auto&& minimap1 = PlayScene::Instance().GetMiniMap();
			if (minimap1.IsShowEnemies() == false) return;
			minimap1.SetShowEnemies(false);
		});
		return true;
	}
}