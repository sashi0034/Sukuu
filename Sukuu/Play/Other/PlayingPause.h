#pragma once
#include "CornerButton.h"

namespace Play
{
	class PlayingPause
	{
	public:
		PlayingPause();
		bool IsPaused() const { return m_pauseAllowed && m_paused; }
		void SetAllowed(bool e) { m_pauseAllowed = e; }
		void Update();

	private:
		bool m_paused{};
		bool m_pauseAllowed{};

		Array<CornerButton> m_buttons{};
		int m_cursorIndex{};
	};
}
