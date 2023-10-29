#pragma once
#include "Forward.h"
#include "Gimmick/GimmickGrid.h"
#include "Player.h"
#include "Enemy/EnemyDefinition.h"
#include "Map/MapGrid.h"
#include "Other/TimeLimiter.h"
#include "UI/UiMiniMap.h"
#include "UI/UiTimeLimiter.h"
#include "Util/EffectWrapper.h"

namespace Play
{
	class ITutorialSetting
	{
	public:
		virtual ~ITutorialSetting() = default;
		virtual MapGrid GetMap() const = 0;
		virtual Vec2 InitialPlayerPos() const = 0;
	};

	struct PlaySingletonData
	{
		ITutorialSetting* tutorial{};
		PlayerPersonalData playerPersonal;
		TimeLimiterData timeLimiter;

		bool IsTutorial() const { return tutorial != nullptr; }
	};

	class PlayScene : public ActorBase
	{
	public:
		PlayScene();
		~PlayScene() override;

		void Init(const PlaySingletonData& data);
		void Update() override;
		void Kill() override;

		MapGrid& GetMap();
		const MapGrid& GetMap() const;

		GimmickGrid& GetGimmick();
		const GimmickGrid& GetGimmick() const;

		Player& GetPlayer();
		const Player& GetPlayer() const;

		// EnemyContainer& GetEnemies();
		const EnemyContainer& GetEnemies() const;

		UiTimeLimiter& GetTimeLimiter();
		const UiTimeLimiter& GetTimeLimiter() const;

		UiMiniMap& GetMiniMap();
		const UiMiniMap& GetMiniMap() const;

		EffectWrapper& FgEffect();
		const EffectWrapper& FgEffect() const;

		EffectWrapper& BgEffect();
		const EffectWrapper& BgEffect() const;

		void RequestHitstopping(double time);

		PlaySingletonData CopyData() const;

		static PlayScene& Instance();

	private:
		class Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
