#pragma once

#include "Play/Forward.h"

namespace Play
{
	class UiDangerMarker : public ActorBase
	{
	public:
		UiDangerMarker();

		void Init();

		void Update() override;

		void MarkEnemy(const RectF& rect);

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
