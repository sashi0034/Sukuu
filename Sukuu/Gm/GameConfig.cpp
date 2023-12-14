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
		// TODO
	}

	GameConfig readJson()
	{
		const JSON json{JSON::Load(configPath)};
		if (not json) return {};

		auto config = GameConfig();

		try
		{
			for (const auto& mapping : json[U"gamepad"][U"mapping"].arrayView())
			{
				config.gamepad.mapping[mapping[U"key"].getString()] = fromArrayView<int>(mapping[U"value"].arrayView());
			}
		}
		catch (...) { readError(U"gamepad.mapping"); }

		return config;
	}
}

namespace Gm
{
	void GameConfig::RequestWrite()
	{
		if (s_instance.has_value()) writeJson(s_instance.value());
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
