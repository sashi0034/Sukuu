#pragma once
#include "Forward.h"
#include "Gimmick/GimmickGrid.h"
#include "Player.h"
#include "Enemy/EnemyDefinition.h"
#include "Map/MapGrid.h"
#include "Other/TimeLimiter.h"
#include "UI/UiTimeLimiter.h"

namespace Play
{
	struct PlaySingletonData
	{
		PlayerPersonalData playerPersonal;
		TimeLimiterData timeLimiter;
	};

	class PlayScene : public ActorBase
	{
	public:
		PlayScene();
		~PlayScene() override;

		void Init(const PlaySingletonData& data);
		void Update() override;

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

		PlaySingletonData CopyData() const;

		static PlayScene& Instance();

	private:
		class Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
