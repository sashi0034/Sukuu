#pragma once

namespace BuildInformation
{
	struct VersionData
	{
		int major;
		int minor;
		int patch;

		String ToString() const;
	};

	struct BuildRecord
	{
		VersionData version;
	};

	const BuildRecord& GetRecord();
}
