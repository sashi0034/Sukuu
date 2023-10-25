#pragma once
#include "Forward.h"
#include "Gimmick/GimmickGrid.h"
#include "Player.h"
#include "Map/MapGrid.h"
#include "Other/TimeLimiter.h"

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

		PlaySingletonData CopyData() const;

		static PlayScene& Instance();

	private:
		class Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
