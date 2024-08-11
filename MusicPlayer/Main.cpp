#include "stdafx.h"

namespace
{
	class LineY
	{
	public:
		static constexpr int Height = 40;
		double CurrentLY() const { return 20 + m_y * Height; }

		double NextLY()
		{
			m_y++;
			return CurrentLY();
		}

	private:
		int m_y{-1};
	};

	void musicLoop()
	{
		const Audio music = Dialog::OpenAudio(Audio::Stream);

		double loopBegin = 0;
		double loopEnd = music.lengthSec();

		while (System::Update())
		{
			LineY ly;

			constexpr double labelW = 120.0;
			const double sliderW = Scene::Size().x - labelW;

			double posSec = music.posSec();
			SimpleGUI::Headline(U"{:.1f}"_fmt(posSec), {0, ly.NextLY()});
			if (SimpleGUI::Slider(U"Playing", posSec, 0, music.lengthSec(), {0, ly.NextLY()}, labelW, sliderW))
			{
				music.seekTime(posSec);
			}

			if (posSec >= loopEnd)
			{
				music.seekTime(loopBegin);
			}

			SimpleGUI::Headline(Format(loopBegin), {0, ly.NextLY()});
			if (SimpleGUI::Button(U"- 0.1", {120, ly.CurrentLY()})) loopBegin -= 0.1;
			if (SimpleGUI::Button(U"+ 0.1", {220, ly.CurrentLY()})) loopBegin += 0.1;
			if (SimpleGUI::Slider(U"Loop begin", loopBegin, 0, music.lengthSec(), {0, ly.NextLY()}, labelW, sliderW))
				loopBegin = Math::Floor(loopBegin * 10) / 10;;

			SimpleGUI::Headline(Format(loopEnd), {0, ly.NextLY()});
			if (SimpleGUI::Button(U"- 0.1", {120, ly.CurrentLY()})) loopEnd -= 0.1;
			if (SimpleGUI::Button(U"+ 0.1", {220, ly.CurrentLY()})) loopEnd += 0.1;
			if (SimpleGUI::Slider(U"Loop end", loopEnd, 0, music.lengthSec(), {0, ly.NextLY()}, labelW, sliderW))
				loopEnd = Math::Floor(loopEnd * 10) / 10;

			ly.NextLY();

			if (SimpleGUI::Button(U"Goto 1 sec before loop end", {0, ly.NextLY()}))
			{
				music.seekTime(loopEnd - 1.0);
			}
			if (SimpleGUI::Button(U"Goto 3 sec before loop end", {0, ly.NextLY()}))
			{
				music.seekTime(loopEnd - 3.0);
			}
			if (SimpleGUI::Button(U"Change music", {0, Scene::Size().y - LineY::Height}))
			{
				return;
			}

			if (music.isPlaying() == false) music.play();
		}
	}
}

void Main()
{
	Window::SetTitle(U"MusicPlayer");

	Scene::SetBackground(ColorF{0.3, 0.3, 0.3});

	while (System::Update())
	{
		musicLoop();
	}
}
