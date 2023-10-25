#include "stdafx.h"
#include "UiTimeLimiter.h"

#include "Play/Other/TimeLimiter.h"

struct Play::UiTimeLimiter::Impl
{
	TimeLimiterData m_data{};

	void Update()
	{
	}
};

namespace Play
{
	UiTimeLimiter::UiTimeLimiter() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void UiTimeLimiter::Init(const TimeLimiterData& data)
	{
		p_impl->m_data = data;
	}

	void UiTimeLimiter::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	const TimeLimiterData& UiTimeLimiter::GetData() const
	{
		return p_impl->m_data;
	}
}
