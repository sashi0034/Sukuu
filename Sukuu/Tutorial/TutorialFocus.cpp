#include "stdafx.h"
#include "TutorialFocus.h"

#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"tutorial.focus." + key);
	}
}

struct Tutorial::TutorialFocus::Impl
{
	bool m_show{};
	Vec2 m_pos{};
	double m_time{};

	void Update()
	{
		if (not m_show) return;

		m_time += Scene::DeltaTime();

		const auto thickness = getToml<double>(U"rect_thickness");
		const auto rectSize = getToml<Size>(U"rect_size");
		const auto rect = RectF(m_pos - rectSize / 2, rectSize);
		const double offset = Scene::Time() * getToml<int>(U"dot_speed");
		(void)rect.top().draw(LineStyle::SquareDot(offset), thickness);
		(void)rect.right().draw(LineStyle::SquareDot(offset), thickness);
		(void)rect.bottom().draw(LineStyle::SquareDot(offset), thickness);
		(void)rect.left().draw(LineStyle::SquareDot(offset), thickness);

		const double p = Periodic::Jump0_1(1s);

		TextureAsset(U"👉")
			.scaled(getToml<double>(U"icon_scale"))
			.drawAt(m_pos + getToml<Vec2>(U"icon_offset") + (1 - p) * getToml<Vec2>(U"icon_move"));
	}
};

namespace Tutorial
{
	TutorialFocus::TutorialFocus() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void TutorialFocus::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	double TutorialFocus::OrderPriority() const
	{
		return 500.0;
	}

	void TutorialFocus::Show(const Vec2& pos)
	{
		p_impl->m_show = true;
		p_impl->m_pos = pos;
	}

	void TutorialFocus::Hide()
	{
		p_impl->m_show = false;
	}
}
