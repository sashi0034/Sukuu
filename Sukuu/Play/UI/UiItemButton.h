#pragma once
#include "Play/Gimmick/ConsumableItem.h"
#include "Play/Forward.h"

namespace Play
{
	class UiItemLabel;

	struct ItemButtonParam
	{
		UiItemLabel& label;
		int index;
		Point center;
		ConsumableItem item;
		std::function<bool()> requestUse;
	};

	class UiItemButton : public ActorBase
	{
	public:
		UiItemButton();

		void Tick(const ItemButtonParam& param);

	private:
		class Impl;
		std::shared_ptr<Impl> p_impl;
	};

	class UiItemLabel : public ActorBase
	{
	public:
		UiItemLabel();
		void Update() override;
		void SetCenter(const Vec2& point);
		void ShowMessage(const DrawableText& text);
		void HideMessage();

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
