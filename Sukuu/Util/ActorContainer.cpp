#include "stdafx.h"
#include "ActorContainer.h"

namespace Util
{
	void ActorContainer::sortActorList()
	{
		std::ranges::stable_sort(
			m_actorList,
			[](const std::unique_ptr<IActor>& left, const std::unique_ptr<IActor>& right)
			{
				return left->OrderPriority() < right->OrderPriority();
			});
	}

	void ActorContainer::Update()
	{
		for (int i = m_actorList.size() - 1; i >= 0; --i)
		{
			const auto& actor = m_actorList[i];
			if (actor->IsDead()) m_actorList.erase(m_actorList.begin() + i);
		}

		// 優先度が高いほど後から更新するように並び変える
		sortActorList();

		// 更新
		for (int i = 0; i < m_actorList.size(); ++i)
		{
			auto&& actor = m_actorList[i];
			actor->Update();
		}
	}

	void ActorContainer::Kill()
	{
		for (auto&& child : m_actorList)
		{
			child->Kill();
		}
		m_actorList.clear();
	}
}
