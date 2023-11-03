#pragma once

namespace Play
{
	class PlayBgm : Uncopyable
	{
	public:
		static PlayBgm& Instance();
		PlayBgm();
		void Refresh();

		bool IsPlaying() const;
		void StartPlay();
		void EndPlay();

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};
}
