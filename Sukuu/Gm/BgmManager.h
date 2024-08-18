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
		BgmManager(const BgmManager& other);

		void Refresh();

		void RequestPlay(const BgmInfo& bgm);
		void EndPlay(double fadeTime = 1.0);
		void SetVolumeRate(double rate);

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};

	/// @brief BgmManager::Instance().Refresh() を毎フレーム呼び出すアドオンを初期化する
	void InitBgmManagerAddon();
}
