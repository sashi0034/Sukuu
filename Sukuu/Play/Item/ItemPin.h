#pragma once

#include "Play/Chara/CharaUtil.h"

namespace Play
{
	class ItemPin : public ActorBase
	{
	public:
		ItemPin();
		void Init(const CharaVec2& pos, Dir4Type dir);
		void Update() override;
		double OrderPriority() const override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
