#include "stdafx.h"
#include "ActorBase.h"
#include "ActorContainer.h"

namespace Util
{
	struct ActorBase::Impl
	{
		bool m_isAlive = true;
		std::unique_ptr<ActorContainer> m_children{};

		bool IsDead() const { return not m_isAlive; }

		void Kill()
		{
			m_isAlive = false;
			if (m_children != nullptr)
			{
				m_children->Kill();
			}
			m_children.reset();
		}
	};

	ActorBase::ActorBase() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void ActorBase::Kill()
	{
		p_impl->Kill();
	}

	bool ActorBase::IsDead() const
	{
		return p_impl->IsDead();
	}

	bool ActorBase::HasChildren() const
	{
		return p_impl->m_children.get() != nullptr;
	}

	ActorContainer& ActorBase::AsParent()
	{
		if (p_impl->m_children.get() == nullptr) p_impl->m_children.reset(new ActorContainer());
		return *p_impl->m_children;
	}

	void ActorBase::Update()
	{
		if (p_impl->m_children.get() == nullptr) return;
		p_impl->m_children->Update();
	}

	double ActorBase::OrderPriority() const
	{
		return 0;
	}

	ActorView::ActorView(const ActorBase& actor) :
		p_impl(actor.p_impl.get())
	{
	}

	bool ActorView::IsDead() const
	{
		return p_impl->IsDead();
	}

	ActorContainer& ActorView::AsParent()
	{
		if (p_impl->m_children.get() == nullptr) p_impl->m_children.reset(new ActorContainer());
		return *p_impl->m_children;
	}

	ActorWeak::ActorWeak(const ActorBase& actor) :
		p_impl(actor.p_impl)
	{
	}

	void ActorWeak::Kill()
	{
		if (auto&& p = p_impl.lock()) p->Kill();
	}

	bool ActorWeak::IsDead() const
	{
		if (auto&& p = p_impl.lock()) return p->IsDead();
		return true;
	}
}
