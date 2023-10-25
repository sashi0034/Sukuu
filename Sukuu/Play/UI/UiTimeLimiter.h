#pragma once

#include "Play/Forward.h"
#include "Play/Other/TimeLimiter.h"

namespace Play
{
	// TODO: インターフェースを分ける

	class UiTimeLimiter : public ActorBase
	{
	public:
		UiTimeLimiter();

		void Init(const TimeLimiterData& data);
		void Update() override;

		const TimeLimiterData& GetData() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
