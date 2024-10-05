#pragma once
#include "ActorBase.h"

namespace Util
{
	class ActorContainer
	{
	public:
		void Update();
		void Kill();

		template <typename T>
		T Birth(const T& actor)
		{
			static_assert(std::is_base_of<ActorBase, T>::value);
			m_actorList.emplace_back(std::make_unique<T>(actor));
			return actor;
		}

		std::vector<std::unique_ptr<ActorBase>>& ActorList() { return m_actorList; }
		const std::vector<std::unique_ptr<ActorBase>>& ActorList() const { return m_actorList; }

	private:
		std::vector<std::unique_ptr<ActorBase>> m_actorList{};
		void sortActorList();
	};
}
