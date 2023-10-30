#pragma once

#include "Play/Forward.h"
#include "Play/Chara/CharaUtil.h"
#include "Play/Item/ConsumableItem.h"
#include "Util/ActorContainer.h"

namespace Play
{
	enum class EnemyKind
	{
		SlimeCat,
		Knight,
	};

	int GetEnemyAttackDamage(EnemyKind kind);

	class EnemyBase : public ActorBase
	{
	public:
		EnemyBase() = default;
		~EnemyBase() override = default;
		virtual bool SendDamageCollider(ItemAttackerAffair& attacker, const RectF& collider) = 0;
		virtual const CharaPosition& Pos() const = 0;
	};

	class EnemyContainer : public Array<std::unique_ptr<EnemyBase>>
	{
	public:
		template <typename T>
		const T& Birth(ActorContainer& parent, const T& enemy)
		{
			static_assert(std::is_base_of<EnemyBase, T>::value);
			emplace_back(std::make_unique<T>(parent.Birth(enemy)));
			return enemy;
		}

		void Refresh();

		int SendDamageCollider(ItemAttackerAffair& attacker, const RectF& collider) const;

		void ForEach(const std::function<void(const EnemyBase&)>& func) const;
	};
}
