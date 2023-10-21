#pragma once
#include "ActorBase.h"
#include "CoroTask.h"

namespace Util
{
	class CoroActor : public ActorBase
	{
	public:
		explicit CoroActor();
		explicit CoroActor(const CoroTaskFunc& task);
		// [[nodiscard]] CoroTaskCall& GetTask() const { return *m_task; };
		void Update() override;

	private:
		std::shared_ptr<CoroTaskCall> m_task;
	};
}
