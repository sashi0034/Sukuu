#pragma once
#include "Util/ActorBase.h"

namespace Sukuu
{
	class AssetReloader : public Util::ActorBase
	{
	public:
		AssetReloader();
		void Update() override;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
