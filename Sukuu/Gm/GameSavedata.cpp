#include "stdafx.h"
#include "GameSavedata.h"

namespace
{
	constexpr StringView dataPath = U"savedata.dat";

	using namespace Gm;

	constexpr int alignedSaveDataSize = 1024;

	static_assert(sizeof(GameSavedata) < alignedSaveDataSize);

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

		/*
		 * もし、バージョン互換性の変換処理を挟むならここに記述
		 */

		d.version = BuildInformation::GetRecord().version;

		return GameSavedata(d);
	}

	void StoreSavedata(const GameSavedata& data)
	{
		BinaryWriter writer{dataPath};
		if (not writer) return;
		const auto d = ReservedSavedata(data);
		writer.write(d);
	}
}
