#pragma once

namespace Sukuu
{
	struct GameSavedata
	{
		int bestReached{};
		double completedTime{};
	};

	Optional<GameSavedata> LoadSavedata();
	void SaveSavedata(const GameSavedata& data);
}
