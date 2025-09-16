#include "stdafx.h"
#include "UiDangerMarker.h"

using namespace Play;

struct UiDangerMarker::Impl
{
	Texture m_texture{Icon::CreateImage(0xF0026, 128)};

	void Init()
	{
	}

	void Update(ActorView self)
	{
		m_texture.drawAt(Scene::Center());
	}
};

namespace Play
{
	UiDangerMarker::UiDangerMarker() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void UiDangerMarker::Init()
	{
		p_impl->Init();
	}

	void UiDangerMarker::Update()
	{
		ActorBase::Update();
		p_impl->Update(*this);
	}
}
