#pragma once
#include "PlayCore.h"

namespace Play
{
	class PlayScene : public ActorBase
	{
	public:
		void Init(const PlaySingletonData& data);
		void Update() override;

		PlayCore& GetCore();
		const PlayCore& GetCore() const;

		static PlayScene Empty();
		static PlayScene Create();

	private:
		PlayScene() = default;

		class Impl;
		std::shared_ptr<Impl> p_impl{};
	};
}
