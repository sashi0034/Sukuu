#pragma once
#include "PlayCore.h"

namespace Play
{
	class PlayScene : public ActorBase
	{
	public:
		PlayScene();
		void Init(const PlaySingletonData& data);
		void Update() override;

		PlayCore& GetCore();
		const PlayCore& GetCore() const;

	private:
		class Impl;
		std::shared_ptr<Impl> p_impl{};
	};
}
