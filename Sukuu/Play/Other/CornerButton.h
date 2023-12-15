#pragma once

namespace Play
{
	void DrawButtonFrame(const RectF& region);

	class CornerButton
	{
	public:
		CornerButton(const StringView& label, const std::function<void()>& action) :
			label(label),
			action(action) { return; }

		using Hovered = bool;
		Hovered Update(int index, int cursorIndex);

	private:
		StringView label;
		std::function<void()> action;
	};
}
