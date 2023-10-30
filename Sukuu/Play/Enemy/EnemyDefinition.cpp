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
		case EnemyKind::SlimeCat:
			return getToml<int>(U"attacker_slime_cat");
		case EnemyKind::Knight:
			return getToml<int>(U"attacker_knight");
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
			if (enemy->SendDamageCollider(attacker, collider)) count++;
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
