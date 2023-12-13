#include "stdafx.h"
#include "PlayScene.h"

class Play::PlayScene::Impl
{
public:
	PlayOperationCore m_core{};
};

namespace Play
{
	PlayScene::PlayScene() : p_impl(std::make_shared<Impl>())
	{
	}

	void PlayScene::Init(const PlaySingletonData& data)
	{
		p_impl->m_core.Init(data);
	}

	void PlayScene::Update()
	{
		p_impl->m_core.Update();
	}

	PlayCore& PlayScene::GetCore()
	{
		return p_impl->m_core;
	}

	const PlayCore& PlayScene::GetCore() const
	{
		return p_impl->m_core;
	}
}
