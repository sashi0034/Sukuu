#pragma once
#include "BuildInformation.h"
#include "GameConfig.h"

#include "Constants.h"

namespace Gm
{
	struct ReachedRecord
	{
		int bestReached{};
		double completedTime{};
	};

	struct TemporaryRecord
	{
		int floorIndex{};
		std::array<double, Constants::MaxFloor_50 + 1> measuredSeconds{};
	};

	/// @brief セーブデータ
	/// これをバイナリとしてファイルに保存するため、メンバの順番を変えてはいけない
	struct GameSavedata
	{
		/// @brief 0. バージョン情報
		BuildInformation::VersionData version{};

		/// @brief 1. 通常モードの記録
		ReachedRecord standard_record{};

		/// @brief 2. 裏モードの記録
		ReachedRecord ura_record{};

		/// @brief 3. クライアントの言語 (Steam で設定された言語を反映するための記録)
		GameLanguage steamLanguage{GameLanguage::Max};

		/// @brief 4. 通常モードの一時記録
		TemporaryRecord standard_temporary{};

		/// @brief 5. 裏モードの一時記録
		TemporaryRecord ura_temporary{};

		ReachedRecord& GetReached(bool isPlayingUra) { return isPlayingUra ? ura_record : standard_record; }

		const ReachedRecord& GetReached(bool isPlayingUra) const { return isPlayingUra ? ura_record : standard_record; }

		TemporaryRecord& GetTemporary(bool isPlayingUra)
		{
			return isPlayingUra ? ura_temporary : standard_temporary;
		}

		const TemporaryRecord& GetTemporary(bool isPlayingUra) const
		{
			return isPlayingUra ? ura_temporary : standard_temporary;
		}
	};

	Optional<GameSavedata> LoadSavedata();
	void StoreSavedata(const GameSavedata& data);
}
