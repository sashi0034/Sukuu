#include "stdafx.h"
#include "ActorBase.h"
#include "ActorContainer.h"

namespace Util
{
	struct ActorBase::Impl
	{
		bool isActive = true;
		bool isAlive = true;
		std::unique_ptr<ActorContainer> children{};
	};

	ActorBase::ActorBase() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void ActorBase::Kill()
	{
		p_impl->isAlive = false;
	}

	bool ActorBase::IsDead() const
	{
		return !p_impl->isAlive;
	}

	bool ActorBase::HasChildren() const
	{
		return p_impl->children.get() != nullptr;
	}

	ActorContainer& ActorBase::AsParent()
	{
		if (p_impl->children.get() == nullptr) p_impl->children.reset(new ActorContainer());
		return *p_impl->children;
	}

	void ActorBase::Update()
	{
		if (p_impl->children.get() == nullptr) return;
		p_impl->children->Update();
	}

	double ActorBase::OrderPriority() const
	{
		return 0;
	}

	void ActorBase::SetActive(bool isActive)
	{
		p_impl->isActive = isActive;
	}

	bool ActorBase::IsActive() const
	{
		return p_impl->isActive;
	}
}
