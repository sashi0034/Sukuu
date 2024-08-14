#pragma once
#include "BuildInformation.h"
#include "GameConfig.h"

namespace Gm
{
	struct ReachedRecord
	{
		int bestReached{};
		double completedTime{};
	};

	/// @brief セーブデータ
	/// これをバイナリとしてファイルに保存するため、メンバの順番を変えてはいけない
	struct GameSavedata
	{
		/// @brief 0. バージョン情報
		BuildInformation::VersionData version{};

		/// @brief 1. 通常モードの記録
		ReachedRecord standard{};

		/// @brief 2. 裏モードの記録
		ReachedRecord ura{};

		/// @brief 3. クライアントの言語 (Steam で設定された言語を反映するための記録)
		GameLanguage steamLanguage{GameLanguage::Max};

		ReachedRecord& GetRecord(bool isPlayingUra) { return isPlayingUra ? ura : standard; }
		const ReachedRecord& GetRecord(bool isPlayingUra) const { return isPlayingUra ? ura : standard; }
	};

	Optional<GameSavedata> LoadSavedata();
	void StoreSavedata(const GameSavedata& data);
}
