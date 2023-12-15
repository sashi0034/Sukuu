#include "stdafx.h"
#include "GameSavedata.h"

namespace
{
	constexpr StringView dataPath = U"savedata.dat";

	using namespace Gm;

	constexpr int alignedSaveDataSize = 1024;

	struct ReservedSavedata : GameSavedata
	{
		uint8 reserved[alignedSaveDataSize - sizeof(GameSavedata)]{};
	};

	static_assert(sizeof(ReservedSavedata) == alignedSaveDataSize);
}

namespace Gm
{
	Optional<GameSavedata> LoadSavedata()
	{
		BinaryReader reader{dataPath};
		if (not reader) return none;
		ReservedSavedata d;
		reader.read(d);
		return GameSavedata(d);
	}

	void SaveSavedata(const GameSavedata& data)
	{
		BinaryWriter writer{dataPath};
		if (not writer) return;
		const auto d = ReservedSavedata(data);
		writer.write(d);
	}
}
