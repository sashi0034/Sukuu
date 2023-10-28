#pragma once

#include "Play/Chara/CharaUtil.h"

namespace Play
{
	class ItemMine : public ActorBase
	{
	public:
		ItemMine();
		void Init(const CharaVec2& pos);
		void Update() override;
		double OrderPriority() const override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
