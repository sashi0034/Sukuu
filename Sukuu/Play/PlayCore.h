﻿#pragma once

#include "Constants.h"
#include "Forward.h"
#include "Gimmick/GimmickGrid.h"
#include "Player.h"
#include "Enemy/EnemyDefinition.h"
#include "Map/MapGrid.h"
#include "Other/TimeLimiter.h"
#include "UI/UiDashKeep.h"
#include "UI/UiMiniMap.h"
#include "UI/UiTimeLimiter.h"
#include "Util/EffectWrapper.h"

namespace Play
{
	struct TutorialPlayerService
	{
		bool canMove;
		bool canScoop;
		std::function<void(const CharaVec2& pos, bool isRunning)> onMoved;
		std::function<void(const CharaVec2& pos)> onScooped;
		std::function<bool(const CharaVec2& pos)> canMoveTo;
		std::function<bool(const CharaVec2& pos)> canScoopTo;
	};

	class ITutorialSetting
	{
	public:
		virtual ~ITutorialSetting() = default;
		virtual MapGrid GetMap() const = 0;
		virtual Vec2 InitialPlayerPos() const = 0;
		virtual const TutorialPlayerService& PlayerService() const = 0;
	};

	class MeasuredSecondsArray : public std::array<double, Constants::MaxFloorIndex + 1>
	{
	public:
		double Sum() const { return SumArrayValues<double>(*this); }
	};

	struct PlaySingletonData
	{
		ITutorialSetting* tutorial{};
		int floorIndex{};
		MeasuredSecondsArray measuredSeconds{};
		PlayerPersonalData playerPersonal{};
		TimeLimiterData timeLimiter{};
		bool dashKeeping{};

		bool IsTutorial() const { return tutorial != nullptr; }
	};

	class PlayCore
	{
	public:
		PlayCore();
		~PlayCore();

		ActorContainer& AsMainContent();

		ActorWeak StartTransition(int floorIndex);
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

		ITutorialSetting* Tutorial();
		const ITutorialSetting* Tutorial() const;

		void RequestHitstopping(double time);

		PlaySingletonData CopyData() const;

		static PlayCore& Instance();

	protected:
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