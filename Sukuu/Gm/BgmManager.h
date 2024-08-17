#pragma once

namespace Gm
{
	struct BgmInfo
	{
		AssetNameView music;
		double loopBegin;
		double loopEnd;
	};

	class BgmManager : Uncopyable
	{
	public:
		static BgmManager& Instance();
		BgmManager();
		void Refresh();

		void RequestPlay(const BgmInfo& bgm);
		void EndPlay(double fadeTime = 1.0);
		void SetVolumeRate(double rate);

		/// @brief 音量レートを強制的に上書きする.
		/// チュートリアルといった特殊シーンで使用.
		void OverrideVolumeRate(Optional<double> rate);

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
