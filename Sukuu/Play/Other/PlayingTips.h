#pragma once

namespace Play
{
	/// @brief TIPS のキーを取得する。これを LocalizedText で変換する必要がある
	StringView GetPlayingTips(int floorIndex);

	Array<StringView> GetAllPlayingTips();
}
