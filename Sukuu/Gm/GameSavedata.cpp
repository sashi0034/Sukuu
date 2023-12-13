#include "stdafx.h"
#include "GameSavedata.h"

namespace
{
	constexpr StringView dataPath = U"savedata.dat";

	using namespace Gm;

	constexpr int alignedSaveDataSize = 1024;

	struct AlignedSavedata : GameSavedata
	{
		uint8 padding[alignedSaveDataSize - sizeof(GameSavedata)]{};
	};

	static_assert(sizeof(AlignedSavedata) == alignedSaveDataSize);
}

namespace Gm
{
	Optional<GameSavedata> LoadSavedata()
	{
		BinaryReader reader{dataPath};
		if (not reader) return none;
		AlignedSavedata d;
		reader.read(d);
		return GameSavedata(d);
	}

	void SaveSavedata(const GameSavedata& data)
	{
		BinaryWriter writer{dataPath};
		if (not writer) return;
		const auto d = AlignedSavedata(data);
		writer.write(d);
	}
}
