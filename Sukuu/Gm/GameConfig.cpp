#include "stdafx.h"
#include "GameConfig.h"

namespace
{
	constexpr StringView configPath = U"config.json";

	using namespace Gm;
	Optional<GameConfig> s_instance{};

	template <typename T>
	Array<T> fromArrayView(JSONArrayView arrayView)
	{
		Array<int> a{};
		for (auto&& v : arrayView) a.push_back(v.get<T>());
		return a;
	}

	void readError(StringView labelView)
	{
#if _DEBUG
		System::MessageBoxOK(U"Failed to load config: " + labelView, MessageBoxStyle::Error);
#endif
	}

	void writeJson(const GameConfig& config)
	{
		JSON json{};

		Array<JSON> gamepadMapping{};
		for (auto&& map : config.gamepad_mapping)
		{
			auto e = JSON();
			e[U"key"] = map.first;
			e[U"value"] = map.second;
			gamepadMapping.push_back(e);
		}
		json[U"gamepad_mapping"] = Format(gamepadMapping);

		if (not json.save(configPath))
		{
			System::MessageBoxOK(U"Failed to write " + configPath, MessageBoxStyle::Error);
		}
	}

	Array<int> parseArrayInt(StringView str)
	{
		Array<int> result{};
		String temp{};
		const auto flash = [&]()
		{
			if (temp.isEmpty()) return;
			result.push_back(Parse<int>(temp));
			temp.clear();
		};
		for (const auto c : str)
		{
			if (U'0' <= c && c <= U'9') temp += c;
			else flash();
		}
		flash();
		return result;
	}

	GameConfig readJson()
	{
		const JSON json{JSON::Load(configPath)};
		if (not json) return {};

		auto config = GameConfig();

		try
		{
			for (const auto& mapping : json[U"gamepad_mapping"].arrayView())
			{
				config.gamepad_mapping[mapping[U"key"].getString()] = parseArrayInt(mapping[U"value"].getString());
			}
		}
		catch (...) { readError(U"gamepad_mapping"); }

		return config;
	}
}

namespace Gm
{
	void GameConfig::RequestWrite()
	{
		writeJson(*this);
	}

	GameConfig& GameConfig::Instance()
	{
		if (not s_instance)
		{
			s_instance = readJson();
		}
		return s_instance.value();
	}
}
