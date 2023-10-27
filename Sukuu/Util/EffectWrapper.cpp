#include "stdafx.h"
#include "EffectWrapper.h"

#include "ActorContainer.h"

struct Util::EffectWrapper::Impl
{
	double m_orderPriority{};
	Effect m_effect{};
};

namespace Util
{
	EffectWrapper::EffectWrapper() : EffectWrapper(0)
	{
	}

	EffectWrapper::EffectWrapper(double orderPriority) :
		p_impl(std::make_shared<Impl>())
	{
		p_impl->m_orderPriority = orderPriority;
	}

	void EffectWrapper::Update()
	{
		ActorBase::Update();
		p_impl->m_effect.update();
	}

	double EffectWrapper::OrderPriority() const
	{
		return p_impl->m_orderPriority;
	}

	void EffectWrapper::add(std::unique_ptr<IEffect> effect)
	{
		p_impl->m_effect.add(std::move(effect));
	}

	void EffectWrapper::add(const ActorBase& effect)
	{
		AsParent().Birth(effect);
	}

	Effect& EffectWrapper::GetEffect()
	{
		return p_impl->m_effect;
	}

	const Effect& EffectWrapper::GetEffect() const
	{
		return p_impl->m_effect;
	}
}
