#include "stdafx.h"
#include "EnemyDefinition.h"

#include "Util/ActorContainer.h"

namespace Play
{
	void EnemyContainer::Refresh()
	{
		for (int i = this->size() - 1; i >= 0; --i)
		{
			if (this->operator[](i)->IsDead())
				this->remove_at(i);
		}
	}

	int EnemyContainer::SendDamageCollider(const RectF& collider) const
	{
		int count{};
		for (auto&& enemy : *this)
		{
			if (enemy->IsDead()) continue;
			if (enemy->SendDamageCollider(collider)) count++;
		}
		return count;
	}
}
