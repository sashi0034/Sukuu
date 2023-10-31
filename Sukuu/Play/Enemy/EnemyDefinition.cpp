#include "stdafx.h"
#include "EnemyDefinition.h"

#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	static inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.enemy." + key);
	}
}

namespace Play
{
	int GetEnemyAttackDamage(EnemyKind kind)
	{
		switch (kind)
		{
		case EnemyKind::SlimeCat: [[ fallthrough]];
		case EnemyKind::SlimeCat_prime:
			return getToml<int>(U"attacker_slime_cat");
		case EnemyKind::Knight: [[fallthrough]];
		case EnemyKind::Knight_prime:
			return getToml<int>(U"attacker_knight");
		case EnemyKind::Catfish:
			return getToml<int>(U"attacker_catfish");
		case EnemyKind::Crab:
			return getToml<int>(U"attacker_crab");
		case EnemyKind::Lion:
			return getToml<int>(U"attacker_lion");
		default: ;
			return 1;
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
			if (enemy->SendDamageCollider(attacker, collider))
			{
				attacker.IncAttacked();
				count++;
			}
		}
		return count;
	}

	void EnemyContainer::ForEach(const std::function<void(const EnemyBase&)>& func) const
	{
		for (auto& enemy : *this)
		{
			if (enemy->IsDead()) continue;
			func(*enemy);
		}
	}
}
