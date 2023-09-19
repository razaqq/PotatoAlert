// Copyright 2020 <github.com/razaqq>

#include "Client/AppDirectories.hpp"
#include "Client/Config.hpp"
#include "Client/Game.hpp"

#include "Core/Encoding.hpp"
#include "Core/File.hpp"
#include "Core/Json.hpp"
#include "Core/Log.hpp"
#include "Core/Process.hpp"
#include "Core/Singleton.hpp"
#include "Core/StandardPaths.hpp"

#include <QDir>
#include <QStandardPaths>

#include <filesystem>
#include <iostream>
#include <string>
#include <utility>


namespace fs = std::filesystem;

using PotatoAlert::Client::Config;
using PotatoAlert::Client::ConfigKey;
using PotatoAlert::Client::Game::GetGamePath;

namespace {

static rapidjson::Document g_defaultConfig = rapidjson::Document(rapidjson::kObjectType);
static std::unordered_map<ConfigKey, std::string_view> g_keyNames =
{
	{ ConfigKey::StatsMode,                "stats_mode" },
	{ ConfigKey::TeamDamageMode,           "team_damage_mode" },
	{ ConfigKey::TeamWinRateMode,          "team_win_rate_mode" },
	{ ConfigKey::TableLayout,              "table_layout" },
	{ ConfigKey::UpdateNotifications,      "update_notifications" },
	{ ConfigKey::MinimizeTray,             "minimize_tray" },
	{ ConfigKey::MatchHistory,             "match_history" },
	{ ConfigKey::SaveMatchCsv,             "save_match_csv" },
	{ ConfigKey::WindowHeight,             "window_height" },
	{ ConfigKey::WindowWidth,              "window_width" },
	{ ConfigKey::WindowX,                  "window_x" },
	{ ConfigKey::WindowY,                  "window_y" },
	{ ConfigKey::WindowState,              "window_state" },
	{ ConfigKey::GameDirectory,            "game_directory" },
	{ ConfigKey::Language,                 "language" },
	{ ConfigKey::MenuBarLeft,              "menubar_left" }
};

}


Config::Config(const fs::path& filePath) : m_filePath(filePath)
{
	g_defaultConfig.SetObject();
	auto a = g_defaultConfig.GetAllocator();
	g_defaultConfig.AddMember(Core::ToRef(g_keyNames[ConfigKey::StatsMode]), ToJson(StatsMode::Pvp), a);
	g_defaultConfig.AddMember(Core::ToRef(g_keyNames[ConfigKey::TeamDamageMode]), ToJson(TeamStatsMode::Average), a);
	g_defaultConfig.AddMember(Core::ToRef(g_keyNames[ConfigKey::TeamWinRateMode]), ToJson(TeamStatsMode::Average), a);
	g_defaultConfig.AddMember(Core::ToRef(g_keyNames[ConfigKey::TableLayout]), ToJson(TableLayout::Horizontal), a);
	g_defaultConfig.AddMember(Core::ToRef(g_keyNames[ConfigKey::UpdateNotifications]), true, a);
	g_defaultConfig.AddMember(Core::ToRef(g_keyNames[ConfigKey::MinimizeTray]), false, a);
	g_defaultConfig.AddMember(Core::ToRef(g_keyNames[ConfigKey::MatchHistory]), true, a);
	g_defaultConfig.AddMember(Core::ToRef(g_keyNames[ConfigKey::SaveMatchCsv]), false, a);
	g_defaultConfig.AddMember(Core::ToRef(g_keyNames[ConfigKey::WindowHeight]), 450, a);
	g_defaultConfig.AddMember(Core::ToRef(g_keyNames[ConfigKey::WindowWidth]), 1500, a);
	g_defaultConfig.AddMember(Core::ToRef(g_keyNames[ConfigKey::WindowX]), 0, a);
	g_defaultConfig.AddMember(Core::ToRef(g_keyNames[ConfigKey::WindowY]), 0, a);
	g_defaultConfig.AddMember(Core::ToRef(g_keyNames[ConfigKey::WindowState]), Qt::WindowState::WindowActive, a);
	std::string gamePathStr = "";
	if (const std::optional<fs::path> gamePath = GetGamePath())
	{
		if (Result<std::string> path = Core::PathToUtf8(gamePath.value()))
			gamePathStr = path.value();
	}
	g_defaultConfig.AddMember(Core::ToRef(g_keyNames[ConfigKey::GameDirectory]), gamePathStr, a);
	g_defaultConfig.AddMember(Core::ToRef(g_keyNames[ConfigKey::Language]), 0, a);
	g_defaultConfig.AddMember(Core::ToRef(g_keyNames[ConfigKey::MenuBarLeft]), true, a);

	if (!Exists())
	{
		if (!CreateDefault())
		{
			LOG_ERROR("Failed to create default config.");
			Core::ExitCurrentProcessWithError(1);
		}
	}
	Load();
	ApplyUpdates();
}

Config::~Config()
{
	Save();
	m_file.Close();
}

void Config::Load()
{
	LOG_TRACE("Trying to Load config...");

	if (m_file)
	{
		m_file.Close();
	}

	m_file = File::Open(m_filePath, File::Flags::Open | File::Flags::Read | File::Flags::Write);
	if (!m_file)
	{
		LOG_ERROR("Failed to open config file: {}", File::LastError());
		Core::ExitCurrentProcessWithError(1);
		return;
	}

	std::string str;
	if (!m_file.ReadAllString(str))
	{
		LOG_ERROR("Failed to read config file: {}", File::LastError());
		Core::ExitCurrentProcessWithError(1);
		return;
	}

	PA_TRY_OR_ELSE(json, Core::ParseJson(str),
	{
		LOG_ERROR("Failed to Parse config as JSON.");

		m_file.Close();
		CreateBackup();

		if (CreateDefault())
			return Load();
		else
			Core::ExitCurrentProcessWithError(1);
	});

	m_json = std::move(json);

	AddMissingKeys();
	CheckTypes();
	CheckEnums();

	LOG_TRACE("Config loaded.");
}

bool Config::Save() const
{
	LOG_TRACE("Saving Config");
	if (!m_file)
	{
		LOG_ERROR("Cannot save config, because file is not open.");
		return false;
	}

	rapidjson::StringBuffer stringBuffer;
	rapidjson::PrettyWriter writer(stringBuffer);
	m_json.Accept(writer);
	
	if (!m_file.WriteString(std::string_view(stringBuffer.GetString(), stringBuffer.GetSize())))
	{
		LOG_ERROR("Failed to write config file: {}", File::LastError());
		return false;
	}
	m_file.FlushBuffer();
	return true;
}

bool Config::Exists() const
{
	std::error_code ec;
	bool exists = fs::exists(m_filePath, ec);
	if (ec)
	{
		LOG_ERROR("Error while checking config existence: {}", ec.message());
		return false;
	}

	if (exists)
	{
		bool regularFile = fs::is_regular_file(m_filePath, ec);
		if (ec)
		{
			LOG_ERROR("Error while checking if config is regular file: {}", ec.message());
			return false;
		}
		return regularFile;
	}
	return false;
}

bool Config::CreateDefault()
{
	LOG_INFO("Creating new default config.");

	if (m_file)
	{
		m_file.Close();
	}

	m_file = File::Open(m_filePath, File::Flags::Create | File::Flags::Read | File::Flags::Write);
	if (!m_file)
	{
		LOG_ERROR("Failed to open config file: {}", File::LastError());
		return false;
	}

	m_json.CopyFrom(g_defaultConfig, m_json.GetAllocator());

	bool saved = Save();
	m_file.Close();
	return saved;
}

bool Config::CreateBackup() const
{
	LOG_INFO("Creating config backup");
	std::error_code ec;
	bool exists = fs::exists(m_filePath, ec);
	if (ec)
	{
		LOG_ERROR("Failed to check for config backup: {}", ec.message());
		return false;
	}

	if (exists)
	{
		fs::path backupConfig = m_filePath;
		backupConfig.replace_extension(".bak");

		// check if backup exists
		exists = fs::exists(backupConfig, ec);
		if (ec)
		{
			LOG_ERROR("Failed to check for config backup: {}", ec.message());
			return false;
		}

		// remove backup if it exists
		if (!exists)
		{
			fs::remove(backupConfig, ec);
			if (ec)
			{
				LOG_ERROR("Failed to remove config backup: {}", ec.message());
				return false;
			}
		}

		// create backup
		fs::rename(m_filePath, backupConfig, ec);
		if (ec)
		{
			LOG_ERROR("Failed to create config backup: {}", ec.message());
			return false;
		}
	}
	return true;
}

void Config::AddMissingKeys()
{
	for (auto it = g_defaultConfig.MemberBegin(); it != g_defaultConfig.MemberEnd(); ++it)
	{
		if (!m_json.HasMember(it->name))
		{
			LOG_INFO("Adding missing key '{}' to config.", it->name.GetString());
			m_json.AddMember(it->name, it->value, m_json.GetAllocator());
		}
	}
}

void Config::CheckTypes()
{
	for (const auto [key, name] : g_keyNames)
	{
		if (!g_defaultConfig.HasMember(name.data()))
			continue;

		if (IsType(key, ConfigType::String) && !m_json[name.data()].IsString()
			|| IsType(key, ConfigType::Bool) && !m_json[name.data()].IsBool()
			|| IsType(key, ConfigType::Int) && !m_json[name.data()].IsInt()
			|| IsType(key, ConfigType::Float) && !m_json[name.data()].IsFloat()
			|| IsType(key, ConfigType::Path) && !m_json[name.data()].IsString()
			|| IsType(key, ConfigType::StatsMode) && !m_json[name.data()].IsString()
			|| IsType(key, ConfigType::TableLayout) && !m_json[name.data()].IsString()
			|| IsType(key, ConfigType::TeamStatsMode) && !m_json[name.data()].IsString())
		{
			LOG_WARN("Config key '{}' was not of expected type, setting to default value", name);
			m_json[name.data()] = g_defaultConfig[name.data()];
		}
	}
}

void Config::CheckEnums()
{
	for (const auto [key, name] : g_keyNames)
	{
		if (!g_defaultConfig.HasMember(name.data()))
			continue;

		if (IsType(key, ConfigType::StatsMode))
		{
			StatsMode statsMode;
			if (!FromJson(m_json[name.data()], statsMode))
			{
				LOG_WARN("Invalid enum value in config for key '{}', setting to default", name.data());
				SetDefault(key);
			}
		}

		if (IsType(key, ConfigType::TeamStatsMode))
		{
			TeamStatsMode teamStatsMode;
			if (!FromJson(m_json[name.data()], teamStatsMode))
			{
				LOG_WARN("Invalid enum value in config for key '{}', setting to default", name.data());
				SetDefault(key);
			}
		}

		if (IsType(key, ConfigType::TableLayout))
		{
			TableLayout tableLayout;
			if (!FromJson(m_json[name.data()], tableLayout))
			{
				LOG_WARN("Invalid enum value in config for key '{}', setting to default", name.data());
				SetDefault(key);
			}
		}
	}
}

void Config::SetDefault(ConfigKey key)
{
	const std::string_view name = GetKeyName(key);
	m_json[name.data()] = g_defaultConfig[name.data()];
}

void Config::ApplyUpdates()
{
	auto renameKey = [&](std::string_view from, std::string_view to)
	{
		if (auto it = m_json.FindMember(from.data()); it != m_json.MemberEnd())
		{
			it->name.SetString(to.data(), to.size());
		}
	};
	renameKey("game_folder", g_keyNames[ConfigKey::GameDirectory]);
	renameKey("menubar_leftside", g_keyNames[ConfigKey::MenuBarLeft]);
	m_json.RemoveMember("api_key");
	m_json.RemoveMember("use_ga");
	m_json.RemoveMember("save_csv");
}

std::string_view Config::GetKeyName(ConfigKey key)
{
	return g_keyNames[key];
}
