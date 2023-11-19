#pragma once

#include "Play/Forward.h"

namespace Play
{
	class PlayBgm : Uncopyable
	{
	public:
		static PlayBgm& Instance();
		PlayBgm();
		void Refresh();

		void RequestPlay(const BgmInfo& bgm);
		void EndPlay();
		void SetVolumeRate(double rate);

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
