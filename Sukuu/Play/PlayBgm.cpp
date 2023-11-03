#include "stdafx.h"
#include "PlayBgm.h"

#include "Assets.generated.h"

namespace
{
	using namespace Play;

	PlayBgm s_instance{};

	std::array bgmList = {
		AssetBgms::tokeitou,
		AssetBgms::kazegasane,
		AssetBgms::obake_dance,
	};

	constexpr double baseVolume = 0.8;
}

struct PlayBgm::Impl
{
	bool m_playing{};
	int m_bgmIndex{};
	double m_volumeRate{1.0};
	double m_targetVolumeRate{1.0};

	void Refresh()
	{
		if (not m_playing) return;

		m_volumeRate = Math::Lerp(m_volumeRate, m_targetVolumeRate, Scene::DeltaTime() * 5.0);

		const auto current = AudioAsset(bgmList[m_bgmIndex]);
		(void)current.setVolume(m_volumeRate * baseVolume);
		// 数秒後に止まるなら、現在の曲をストップして次に進む
		if (current.posSec() > current.lengthSec() - 3.0)
		{
			current.stop(3.0s);
			m_bgmIndex = (m_bgmIndex + 1) % bgmList.size();
			AudioAsset(bgmList[m_bgmIndex]).play(2.0s);
		}
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

	bool PlayBgm::IsPlaying() const
	{
		return p_impl->m_playing;
	}

	PlayBgm::PlayBgm() :
		p_impl(std::make_shared<Impl>())
	{
	}

	void PlayBgm::StartPlay()
	{
		p_impl->m_volumeRate = 1.0;
		p_impl->m_targetVolumeRate = 1.0;
		p_impl->m_playing = true;
		AudioAsset(bgmList[p_impl->m_bgmIndex]).play();
	}

	void PlayBgm::EndPlay()
	{
		p_impl->m_playing = false;
		AudioAsset(bgmList[p_impl->m_bgmIndex]).stop(1.0s);
	}

	void PlayBgm::SetVolumeRate(double rate)
	{
		p_impl->m_targetVolumeRate = rate;
	}
}
