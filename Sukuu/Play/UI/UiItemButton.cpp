﻿#include "stdafx.h"
#include "UiItemButton.h"

#include "AssetKeys.h"
#include "Util/CoroActor.h"
#include "Util/CoroUtil.h"
#include "Util/EasingAnimation.h"

class Play::UiItemButton::Impl
{
public:
	bool m_enteredBefore{};
	double m_scale{1};

	void Tick(ActorBase& self, const ItemButtonParam& param)
	{
		const int w = 80 * m_scale;
		constexpr int r = 10;

		auto&& itemProps = GetItemProps(param.item);
		const bool isItemContaining = param.item != ConsumableItem::None;

		const auto rect = RoundRect{param.center - Point{w / 2, w / 2}, w, w, r};
		const bool entered = rect.rect.mouseOver();
		(void)rect
		      .drawShadow(Vec2{6, 6}, 24, 3)
		      .draw(Color{U"#3b3b3b"}.lerp(Palette::White, entered && isItemContaining ? 0.7 : 0.0));
		const auto textPos = param.center - Point{w / 3, w / 2};
		Circle(textPos, w / 4)
			.drawShadow(Vec2{2, 2}, 8, 2)
			.draw(Color{U"#404040"});
		(void)FontAsset(AssetKeys::F24)(U"{}"_fmt(param.index)).drawAt(textPos);

		if (isItemContaining)
			(void)TextureAsset(itemProps.emoji).resized(Vec2{w, w} * 0.8f).drawAt(
				param.center, ColorF(entered ? 0.7 : 1.0));

		const bool justUsed = entered && MouseL.down()
			                      ? param.requestUse()
			                      : false;

		if (entered && not m_enteredBefore)
		{
			m_scale = 1;
			AnimateEasing<BoomerangParabola>(self, &m_scale, 1.1, 0.2);
			if (isItemContaining) param.label.ShowMessage(FontAsset(AssetKeys::F24)(itemProps.desc));
		}
		else if ((not entered && m_enteredBefore) || justUsed)
		{
			m_scale = 1;
			AnimateEasing<BoomerangParabola>(self, &m_scale, 0.9, 0.2);
			param.label.HideMessage();
		}
		m_enteredBefore = entered;
	}
};

namespace Play
{
	UiItemButton::UiItemButton() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void UiItemButton::Tick(const ItemButtonParam& param)
	{
		ActorBase::Update();
		p_impl->Tick(*this, param);
	}

	struct UiItemLabel::Impl
	{
		Vec2 center{};
		DrawableText message{};
		bool isShowing{};
		double scale{};
		CoroActor animation{};
	};

	UiItemLabel::UiItemLabel() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void UiItemLabel::Update()
	{
		ActorBase::Update();
		if (p_impl->isShowing)
		{
			const int w = 800 * p_impl->scale;
			constexpr int h = 40;
			RoundRect(p_impl->center.movedBy({-w / 2, -h / 2}), {w, h}, 10)
				.draw(Arg::top = ColorF{0.3, 0.3, 0.3, 0.7}, Arg::bottom = ColorF{0, 0, 0, 0.3});
			p_impl->message.drawAt(p_impl->center);
		}
	}

	void UiItemLabel::SetCenter(const Vec2& point)
	{
		p_impl->center = point;
	}

	void UiItemLabel::ShowMessage(const DrawableText& text)
	{
		p_impl->animation.Kill();
		p_impl->message = std::move(text);
		p_impl->isShowing = true;
		p_impl->scale = 1;
		AnimateEasing<BoomerangParabola>(*this, &p_impl->scale, 1.1, 0.2);
	}

	void UiItemLabel::HideMessage()
	{
		p_impl->animation.Kill();
		p_impl->animation = StartCoro(*this, [&](YieldExtended yield)
		{
			yield.WaitForDead(AnimateEasing<EaseInBack>(*this, &p_impl->scale, 0.0, 0.2));
			p_impl->isShowing = false;
		});
	}
}