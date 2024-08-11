#pragma once

#include "Constants.h"
#include "Forward.h"
#include "Gimmick/GimmickGrid.h"
#include "Player.h"
#include "Enemy/EnemyDefinition.h"
#include "Map/MapGrid.h"
#include "Other/PlayingPause.h"
#include "Other/TimeLimiter.h"
#include "UI/UiDashKeep.h"
#include "UI/UiMessenger.h"
#include "UI/UiMiniMap.h"
#include "UI/UiTimeLimiter.h"
#include "Util/EffectWrapper.h"

namespace Play
{
	struct BgCustomDrawer;

	struct DesignatedMapInfo
	{
		MapGrid map;
		Vec2 initialPlayerPos;
	};

	class MeasuredSecondsArray : public std::array<double, Constants::MaxFloorIndex + 1>
	{
	public:
		double Sum() const { return SumArrayValues<double>(*this); }
	};

	struct PlaySingletonData
	{
		Optional<DesignatedMapInfo> designatedMap{};
		int floorIndex{};
		MeasuredSecondsArray measuredSeconds{};
		PlayerPersonalData playerPersonal{};
		TimeLimiterData timeLimiter{};
		bool dashKeeping{};
		int itemIndexing{};

		bool IsDesignatedMap() const { return designatedMap.has_value(); }
	};

	class PlayCore
	{
	public:
		static PlayCore Create();
		static PlayCore Empty();
		~PlayCore();

		ActorContainer& AsMainContent();
		ActorContainer& AsUiContent();

		ActorWeak StartTransition(int floorIndex, bool floorDown);
		ActorWeak EndTransition();
		ActorWeak PerformGameOver();

		MapGrid& GetMap();
		const MapGrid& GetMap() const;

		GimmickGrid& GetGimmick();
		const GimmickGrid& GetGimmick() const;

		Player& GetPlayer();
		const Player& GetPlayer() const;

		EnemyContainer& GetEnemies();
		const EnemyContainer& GetEnemies() const;

		UiTimeLimiter& GetTimeLimiter();
		const UiTimeLimiter& GetTimeLimiter() const;

		UiMiniMap& GetMiniMap();
		const UiMiniMap& GetMiniMap() const;

		UiDashKeep& GetDashKeep();
		const UiDashKeep& GetDashKeep() const;

		EffectWrapper& FgEffect();
		const EffectWrapper& FgEffect() const;

		EffectWrapper& BgEffect();
		const EffectWrapper& BgEffect() const;

		PlayingPause& GetPause();
		const PlayingPause& GetPause() const;

		void SetBgCustomDrawer(const BgCustomDrawer& drawer);

		void EnableCaveVision(bool enabled);

		UiMessenger BirthUiMessenger();

		void RequestHitstopping(double time);

		PlaySingletonData CopyData() const;

		static PlayCore& Instance();

	protected:
		explicit PlayCore(bool empty);

		class Impl;
		std::shared_ptr<Impl> p_impl{};
	};

	class PlayOperationCore final : public PlayCore
	{
	public:
		void Init(const PlaySingletonData& data);
		void Update();
	};
}
