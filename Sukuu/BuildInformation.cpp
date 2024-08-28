#include "stdafx.h"
#include "BuildInformation.h"

namespace
{
	using namespace BuildInformation;

	//-----------------------------------------------
	/// @brief ビルド情報
	constexpr BuildRecord buildRecord{
		.version = {2, 0, 1},
	};
	//-----------------------------------------------
}

namespace BuildInformation
{
	String VersionData::ToString() const
	{
		return U"{}.{}.{}"_fmt(major, minor, patch);
	}

	const BuildRecord& GetRecord()
	{
		return buildRecord;
	}
}
