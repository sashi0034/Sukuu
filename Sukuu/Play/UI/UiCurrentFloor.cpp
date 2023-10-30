#include "stdafx.h"
#include "UiCurrentFloor.h"

#include "AssetKeys.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.ui_current_floor." + key);
	}
}

struct Play::UiCurrentFloor::Impl
{
	String m_text{};

	void Update()
	{
		if (m_text.empty()) return;
		FontAsset(AssetKeys::RocknRoll_Bitmap)(m_text)
			.draw(Arg::bottomLeft = Rect(Scene::Size()).bl() + getToml<Point>(U"padding"), Palette::Lavender);
	}
};

namespace Play
{
	UiCurrentFloor::UiCurrentFloor() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void UiCurrentFloor::Init(int floorIndex)
	{
		p_impl->m_text = U"階層 B{}"_fmt(floorIndex);
	}

	void UiCurrentFloor::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}
}
