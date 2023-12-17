#include "stdafx.h"
#include "PlayingUra.h"

namespace
{
	bool s_playingUra{};
}

namespace Play
{
	bool IsPlayingUra()
	{
		return s_playingUra;
	}

	void SetPlayingUra(bool ura)
	{
		s_playingUra = ura;
	}
}
