#pragma once
#include "Forward.h"
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

		static PlayScene& Instance();

	private:
		class Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
