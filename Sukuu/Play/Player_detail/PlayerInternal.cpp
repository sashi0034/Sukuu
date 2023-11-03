#include "stdafx.h"
#include "PlayerInternal.h"

#include "Play/PlayBgm.h"
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

	void UpdatePlayerVision(PlayerVisionState& vision, PlayerAct act)
	{
		const double d = Scene::DeltaTime() * 5.0;
		double radius{1.0};
		double heartbeat{1.0};
		if (act == PlayerAct::Running)
		{
			radius = 0.9;
			heartbeat = 2.1;
		}
		else if (act == PlayerAct::Walk)
		{
			heartbeat = 1.5;
		}

		vision.radiusRate = Math::Lerp(vision.radiusRate, radius, d);
		vision.heartbeatRate = Math::Lerp(vision.heartbeatRate, heartbeat, d);
	}

	void ControlPlayerBgm(const CharaVec2& pos, const MapGrid& map)
	{
		double rate = 1.0;
		const auto currPoint = pos.MapPoint();
		if (not map.Data().inBounds(currPoint) || map.At(currPoint).kind == TerrainKind::Wall)
		{
			// 壁にいる時は、ボリューム下げる
			rate = 0.3;
		}
		PlayBgm::Instance().SetVolumeRate(rate);
	}
}
