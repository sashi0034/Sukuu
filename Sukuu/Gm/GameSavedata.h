#pragma once

namespace Gm
{
	struct GameSavedata
	{
		int bestReached{};
		double completedTime{};
	};

	Optional<GameSavedata> LoadSavedata();
	void SaveSavedata(const GameSavedata& data);
}
