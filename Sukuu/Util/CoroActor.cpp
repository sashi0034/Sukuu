#include "stdafx.h"
#include "CoroActor.h"

namespace Util
{
	CoroActor::CoroActor()
	{
		Kill();
	}

	CoroActor::CoroActor(const CoroTaskFunc& task) :
		m_task(std::make_shared<CoroTaskCall>(task))
	{
	}

	void CoroActor::Update()
	{
		ActorBase::Update();
		if (m_task == nullptr) return;
		if ((*m_task)())
		{
		}
		else
		{
			ActorBase::Kill();
		}
	}
}
