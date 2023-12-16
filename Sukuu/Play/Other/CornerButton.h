#pragma once

namespace Play
{
	void DrawButtonFrame(const RectF& region);

	class CornerButton
	{
	public:
		CornerButton(const StringView& label, const std::function<void()>& action) :
			m_label(label),
			m_action(action) { return; }

		using Hovered = bool;
		Hovered Update(int index, int cursorIndex);

	private:
		StringView m_label;
		std::function<void()> m_action;
	};
}
