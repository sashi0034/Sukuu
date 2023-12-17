#pragma once

namespace Gm
{
	struct ReachedRecord
	{
		int bestReached{};
		double completedTime{};
	};

	struct GameSavedata
	{
		ReachedRecord standard{};
		ReachedRecord ura{};

		ReachedRecord& GetRecord(bool isPlayingUra) { return isPlayingUra ? ura : standard; }
		const ReachedRecord& GetRecord(bool isPlayingUra) const { return isPlayingUra ? ura : standard; }
	};

	Optional<GameSavedata> LoadSavedata();
	void SaveSavedata(const GameSavedata& data);
}
