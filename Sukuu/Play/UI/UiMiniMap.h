#pragma once

#include "Play/Forward.h"

namespace Play
{
	// TODO: インターフェースにする

	class UiMiniMap : public ActorBase
	{
	public:
		UiMiniMap();
		void Init(const Size& mapSize);
		void Update() override;

		bool CanSpotStairsAndAllItems() const;
		void SpotStairsAndAllItems();
		void SetShowEnemies(bool isShow);
		bool IsShowEnemies() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
