#include "stdafx.h"
#include "PlayBgm.h"

#include "Assets.generated.h"
#include "Constants.h"

namespace
{
	using namespace Play;

	PlayBgm s_instance{};

	constexpr double baseVolume = 0.7;
}

struct PlayBgm::Impl
{
	BgmInfo m_musicInfo{};
	double m_volumeRate{1.0};
	double m_targetVolumeRate{1.0};

	void Refresh()
	{
		if (m_musicInfo.music.isEmpty()) return;

		m_volumeRate = Math::Lerp(m_volumeRate, m_targetVolumeRate, Scene::DeltaTime() * 5.0);

		const auto playingMusic = AudioAsset(m_musicInfo.music);
		(void)playingMusic.setVolume(m_volumeRate * baseVolume);

		if (playingMusic.posSec() >= m_musicInfo.loopEnd)
		{
			// ループ処理
			playingMusic.seekTime(m_musicInfo.loopBegin);
		}

		// 再生してなかったら、再生する
		if (playingMusic.isPlaying() == false) playingMusic.play(Constants::BgmMixBus);
	}
};

namespace Play
{
	PlayBgm& PlayBgm::Instance()
	{
		return s_instance;
	}

	void PlayBgm::Refresh()
	{
		p_impl->Refresh();
	}

	PlayBgm::PlayBgm() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void PlayBgm::RequestPlay(const BgmInfo& bgm)
	{
		if (p_impl->m_musicInfo.music == bgm.music) return;

		if (not p_impl->m_musicInfo.music.isEmpty()) AudioAsset(p_impl->m_musicInfo.music).stop(0.5s);
		p_impl->m_musicInfo = bgm;
	}

	void PlayBgm::EndPlay()
	{
		AudioAsset(p_impl->m_musicInfo.music).stop(1.0s);
		p_impl->m_musicInfo.music = {};
	}

	void PlayBgm::SetVolumeRate(double rate)
	{
		p_impl->m_targetVolumeRate = rate;
	}
}
