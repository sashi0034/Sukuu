#include "stdafx.h"
#include "EnemyDefinition.h"

#include "Util/ActorContainer.h"
#include "Util/TomlParametersWrapper.h"

namespace Play
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.enemy." + key);
	}

	double GetEnemyAttackDamage(EnemyKind kind)
	{
		switch (kind)
		{
		case EnemyKind::SlimeCat:
			return getToml<double>(U"attacker_slime_cat");
		case EnemyKind::Knight:
			return getToml<double>(U"attacker_knight");
		default: ;
			return 1.0;
		}
	}

	void EnemyContainer::Refresh()
	{
		for (int i = this->size() - 1; i >= 0; --i)
		{
			if (this->operator[](i)->IsDead())
				this->remove_at(i);
		}
	}

	int EnemyContainer::SendDamageCollider(ItemAttackerAffair& attacker, const RectF& collider) const
	{
		int count{};
		for (auto&& enemy : *this)
		{
			if (enemy->IsDead()) continue;
			if (enemy->SendDamageCollider(attacker, collider)) count++;
		}
		return count;
	}
}
