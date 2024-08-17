#pragma once

#include "Play/Forward.h"

namespace Play
{
	enum class CaveSnowfallKind
	{
		None,
		Flurries,
		Steady,
		Blizzard,
		Photons,
		Lumineer,
	};

	class CaveSnowfall : public ActorBase
	{
	public:
		CaveSnowfall();
		CaveSnowfall(CaveSnowfallKind kind);

		void Update() override;
		double OrderPriority() const override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
