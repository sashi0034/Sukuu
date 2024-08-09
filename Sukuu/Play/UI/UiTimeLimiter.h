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
		double OrderPriority() const override { return -1000; };

		void Damage(double time, bool isEnemyDamage);
		void Heal(double time);
		void ExtendMax(double time);
		void ForceTerminate();

		void SetCountEnabled(bool enabled);
		bool IsCountEnabled() const;
		void SetImmortal(bool immortal);
		void MisrepresentedAsIfZero();

		const TimeLimiterData& GetData() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
