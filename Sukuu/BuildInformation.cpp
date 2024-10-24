#include "stdafx.h"
#include "BuildInformation.h"

namespace
{
	using namespace BuildInformation;

	//-----------------------------------------------
	/// @brief ビルド情報
	constexpr BuildRecord buildRecord{
		.version = {2, 1, 0},
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
