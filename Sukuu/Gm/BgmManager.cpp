#include "stdafx.h"
#include "BgmManager.h"

#include "Constants.h"

namespace
{
	using namespace Gm;

	BgmManager s_instance{};

	constexpr double baseVolume = 0.7;

	class BgmManagerAddon : public IAddon
	{
	public:
		bool update() override
		{
			BgmManager::Instance().Refresh();
			return true;
		}
	};
}

struct BgmManager::Impl
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

namespace Gm
{
	BgmManager& BgmManager::Instance()
	{
		return s_instance;
	}

	void BgmManager::Refresh()
	{
		p_impl->Refresh();
	}

	BgmManager::BgmManager() :
		p_impl(std::make_shared<Impl>())
	{
	}

	BgmManager::BgmManager(const BgmManager& other) :
		p_impl(std::make_shared<Impl>(*other.p_impl))
	{
	}

	void BgmManager::RequestPlay(const BgmInfo& bgm)
	{
		if (p_impl->m_musicInfo.music == bgm.music) return;

		if (not p_impl->m_musicInfo.music.isEmpty()) AudioAsset(p_impl->m_musicInfo.music).stop(0.5s);
		p_impl->m_musicInfo = bgm;
	}

	void BgmManager::EndPlay(double fadeTime)
	{
		AudioAsset(p_impl->m_musicInfo.music).stop(Duration{fadeTime});
		p_impl->m_musicInfo.music = {};
	}

	void BgmManager::SetVolumeRate(double rate)
	{
		p_impl->m_targetVolumeRate = rate;
	}

	void InitBgmManagerAddon()
	{
		Addon::Register<BgmManagerAddon>(U"BgmManagerAddon");
	}
}
