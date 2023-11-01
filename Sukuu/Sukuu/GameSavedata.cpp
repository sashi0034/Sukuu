#include "stdafx.h"
#include "GameSavedata.h"

namespace
{
	constexpr String dataPath = U"savedata.dat";
}

namespace Sukuu
{
	Optional<GameSavedata> LoadSavedata()
	{
		BinaryReader reader{dataPath};
		if (not reader) return none;
		GameSavedata d;
		reader.read(d);
		return d;
	}

	void SaveSaveData(const GameSavedata& data)
	{
		BinaryWriter writer{dataPath};
		if (not writer) return;
		writer.write(data);
	}
}
