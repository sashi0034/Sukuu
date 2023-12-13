#pragma once

#include "Play/Forward.h"
#include "Play/PlayCore.h"

namespace Play
{
	class UiItemContainer : public ActorBase
	{
	public:
		UiItemContainer();
		void Init(int itemIndexing);

		void Update() override;
		int GetIndexing() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
};
