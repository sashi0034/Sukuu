#pragma once
#include "GamepadButton.h"
#include "Util/Utilities.h"

namespace Gm
{
	enum class GameLanguage
	{
		Ja = 0,
		En = 1,
		Cs = 2,
		Ct = 3,
		Ko = 4,
		Max,
	};

	constexpr int GameLanguagesCount = static_cast<int>(GameLanguage::Max);

	const inline std::array<String, GameLanguagesCount> LanguageCodes{
		U"ja",
		U"en",
		U"cs",
		U"ct",
		U"ko",
	};

	const inline std::array<String, GameLanguagesCount> LanguageLabels{
		U"日本語",
		U"English",
		U"简体中文",
		U"繁體中文",
		U"한국어",
	};

	static_assert(LanguageLabels.size() == static_cast<int>(GameLanguage::Max));

	class TenStepNumber : public Util::EnumValue<int>
	{
	public:
		TenStepNumber(int v) : EnumValue(std::max(std::min(v, 10), 0)) { return; }

		double GetRate(double base = 10.0) const
		{
			return value() / base;
		}
	};

	struct GameConfig
	{
		bool fullscreen{true};
		GameLanguage language{};

		TenStepNumber bgm_volume{5};
		TenStepNumber se_volume{5};
		TenStepNumber camera_move{2};

		HashTable<String, GamepadButtonMapping> gamepad_mapping{};

		void ApplySystems();

		void RequestWrite();
		static GameConfig& Instance();
	};
}
