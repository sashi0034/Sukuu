﻿#include "stdafx.h"
#include "GameConfig.h"

#include "Constants.h"

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

	bool isValidGamepadMapping(const GamepadButtonMapping& mapping)
	{
		int zeroCount{};
		for (int i = 0; i < mapping.size(); ++i)
		{
			if (mapping[i] == 0) zeroCount++;
		}

		// 0 が 2 つ以上存在する状況は、2 つめ以降の 0 が未定義ということになるので false にする
		return zeroCount <= 1;
	}

	void writeJson(const GameConfig& config)
	{
		JSON json{};

		json[U"fullscreen"] = config.fullscreen;

		json[U"bgm_volume"] = config.bgm_volume.value();

		json[U"se_volume"] = config.se_volume.value();

		json[U"camera_move_x"] = config.camera_move_x.value();

		json[U"camera_move_y"] = config.camera_move_y.value();

		json[U"language"] = static_cast<int>(config.language);

		Array<JSON> gamepadMapping{};
		for (auto&& map : config.gamepad_mapping)
		{
			if (isValidGamepadMapping(map.second) == false) continue;

			auto e = JSON();
			e[U"key"] = map.first;
			e[U"value"] = map.second;
			gamepadMapping.push_back(e);
		}
		json[U"gamepad_mapping"] = gamepadMapping;

		if (not json.save(configPath))
		{
			System::MessageBoxOK(U"Failed to write " + configPath, MessageBoxStyle::Error);
		}
	}

	void tryRead(StringView labelView, const std::function<void()>& read)
	{
		try
		{
			read();
		}
		catch (...)
		{
#if _DEBUG
			Console.writeln(U"Failed to load config: " + labelView);
#endif
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

		tryRead(U"fullscreen", [&] { config.fullscreen = json[U"fullscreen"].get<bool>(); });

		tryRead(U"bgm_volume", [&] { config.bgm_volume = json[U"bgm_volume"].get<int>(); });

		tryRead(U"se_volume", [&] { config.se_volume = json[U"se_volume"].get<int>(); });

		tryRead(U"camera_move_x", [&] { config.camera_move_x = json[U"camera_move_x"].get<int>(); });

		tryRead(U"camera_move_y", [&] { config.camera_move_y = json[U"camera_move_y"].get<int>(); });

		tryRead(U"language", [&] { config.language = static_cast<GameLanguage>(json[U"language"].get<int>()); });

		tryRead(U"gamepad_mapping", [&]
		{
			for (const auto& mapping : json[U"gamepad_mapping"].arrayView())
			{
				config.gamepad_mapping[mapping[U"key"].getString()] = parseArrayInt(mapping[U"value"].getString());
			}
		});

		return config;
	}
}

namespace Gm
{
	void GameConfig::ApplySystems()
	{
		Window::SetFullscreen(fullscreen);
		GlobalAudio::BusSetVolume(Constants::BgmMixBus, bgm_volume.GetRate(5));
		GlobalAudio::BusSetVolume(MixBus0, se_volume.GetRate(5));
	}

	void GameConfig::RequestWrite()
	{
		writeJson(*this);
	}

	GameConfig& GameConfig::Instance()
	{
		if (not s_instance)
		{
			s_instance = readJson();
			s_instance->ApplySystems();
		}
		return s_instance.value();
	}
}
