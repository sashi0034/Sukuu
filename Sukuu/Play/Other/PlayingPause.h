#pragma once

namespace Play
{
	class PlayingPause
	{
	public:
		bool IsPaused() const { return m_pauseAllowed && m_paused; }
		void SetAllowed(bool e) { m_pauseAllowed = e; }
		void Update();

	private:
		bool m_paused{};
		bool m_pauseAllowed{};
	};
}
