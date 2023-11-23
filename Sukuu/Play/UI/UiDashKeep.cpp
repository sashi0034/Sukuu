#include "stdafx.h"
#include "UiDashKeep.h"

#include "AssetKeys.h"
#include "Util/TomlParametersWrapper.h"

namespace
{
	template <typename T>
	inline T getToml(const String& key)
	{
		return Util::GetTomlParameter<T>(U"play.ui_dash_keep." + key);
	}
}

struct Play::UiDashKeep::Impl
{
	bool m_keeping{};

	void Update()
	{
		const auto center = Scene::Size() + getToml<Point>(U"padding");
		const auto circle = Circle(center, getToml<int>(U"radius"));

		const bool isFocusing = circle.intersects(Cursor::PosF());
		if (isFocusing && MouseL.down())
		{
			m_keeping = not m_keeping;
		}

		Transformer2D transformer2D{Mat3x2::Scale(isFocusing ? 1.1 : 1.0, center)};

		const auto circleColor =
			(m_keeping ? getToml<ColorF>(U"keep_color") : getToml<ColorF>(U"default_color"))
			* (isFocusing ? 0.7 : 1.0);
		(void)circle
		      .drawShadow({0, 4}, 8, 2, circleColor)
		      .draw(circleColor);

		if (m_keeping)
		{
			FontAsset(AssetKeys::RocknRoll_24_Bitmap)(U"ダッシュ")
				.regionAt(center).stretched(4.0).shearedX(12.0).draw(ColorF(0.3, 0.7));
		}
		(void)FontAsset(AssetKeys::RocknRoll_24_Bitmap)(U"ダッシュ")
			.drawAt(center, Palette::White);
	}
};

namespace Play
{
	UiDashKeep::UiDashKeep() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void UiDashKeep::Init(bool isKeeping)
	{
		p_impl->m_keeping = isKeeping;
	}

	void UiDashKeep::Update()
	{
		ActorBase::Update();
		p_impl->Update();
	}

	bool UiDashKeep::IsKeeping() const
	{
		return p_impl->m_keeping;
	}
}
