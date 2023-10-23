#pragma once
#include "Forward.h"
#include "Gimmick.h"
#include "Player.h"
#include "Map/MapGrid.h"

namespace Play
{
	class PlayScene : public ActorBase
	{
	public:
		PlayScene();
		~PlayScene() override;
		void Update() override;

		MapGrid& GetMap();
		const MapGrid& GetMap() const;

		GimmickGrid& GetGimmick();
		const GimmickGrid& GetGimmick() const;

		Player& GetPlayer();
		const Player& GetPlayer() const;

		static PlayScene& Instance();

	private:
		class Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
