// Copyright 2020 <github.com/razaqq>

#include "Client/AppDirectories.hpp"
#include "Client/Game.hpp"

#include "Core/File.hpp"
#include "Core/Json.hpp"
#include "Core/Log.hpp"

#include "Client/Config.hpp"

#include "Core/StandardPaths.hpp"

#include <QApplication>
#include <QDir>
#include <QStandardPaths>

#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <utility>


namespace fs = std::filesystem;

using PotatoAlert::Client::AppDirectories;
using PotatoAlert::Client::Config;
using PotatoAlert::Client::ConfigKey;
using PotatoAlert::Client::Game::GetGamePath;

static json g_defaultConfig;
static std::unordered_map<ConfigKey, std::string> g_keyNames =
{
	{ ConfigKey::StatsMode,                "stats_mode" },
	{ ConfigKey::TeamDamageMode,           "team_damage_mode" },
	{ ConfigKey::TeamWinRateMode,          "team_win_rate_mode" },
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
	{ ConfigKey::OverrideReplaysDirectory, "override_replays_directory" },
	{ ConfigKey::ReplaysDirectory,         "replays_directory" },
	{ ConfigKey::Language,                 "language" },
	{ ConfigKey::MenuBarLeft,              "menubar_left" }
};

Config::Config(const std::string& filePath) : m_filePath(filePath)
{
	g_defaultConfig = {
		{ g_keyNames[ConfigKey::StatsMode],                StatsMode::Pvp },
		{ g_keyNames[ConfigKey::TeamDamageMode],           TeamStatsMode::Average },
		{ g_keyNames[ConfigKey::TeamWinRateMode],          TeamStatsMode::Average },
		{ g_keyNames[ConfigKey::UpdateNotifications],      true },
		{ g_keyNames[ConfigKey::MinimizeTray],             false },
		{ g_keyNames[ConfigKey::MatchHistory],             true },
		{ g_keyNames[ConfigKey::SaveMatchCsv],             false },
		{ g_keyNames[ConfigKey::WindowHeight],             450 },
		{ g_keyNames[ConfigKey::WindowWidth],              1500 },
		{ g_keyNames[ConfigKey::WindowX],                  0 },
		{ g_keyNames[ConfigKey::WindowY],                  0 },
		{ g_keyNames[ConfigKey::WindowState],              Qt::WindowState::WindowActive },
		{ g_keyNames[ConfigKey::GameDirectory],            GetGamePath().value_or("") },
		{ g_keyNames[ConfigKey::OverrideReplaysDirectory], false },
		{ g_keyNames[ConfigKey::ReplaysDirectory],         "" },
		{ g_keyNames[ConfigKey::Language],                 0 },
		{ g_keyNames[ConfigKey::MenuBarLeft],              true }
	};

	if (!Exists())
	{
		if (!CreateDefault())
		{
			LOG_ERROR("Failed to create default config.");
			QApplication::exit(1);
		}
	}
	Load();
	ApplyUpdates();
	AddMissingKeys();
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
		QApplication::exit(1);
	}

	std::string str;
	if (!m_file.ReadAllString(str))
	{
		LOG_ERROR("Failed to read config file: {}", File::LastError());
		QApplication::exit(1);
	}

	sax_no_exception sax(m_json);
	if (!json::sax_parse(str, &sax))
	{
		LOG_ERROR("Failed to Parse config as JSON.");

		m_file.Close();
		CreateBackup();

		if (CreateDefault())
			Load();
		else
			QApplication::exit(1);
	}

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

	if (!m_file.WriteString(m_json.dump(4)))
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
	m_json = g_defaultConfig;

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
	for (auto it = g_defaultConfig.begin(); it != g_defaultConfig.end(); ++it)
	{
		if (!m_json.contains(it.key()))
		{
			LOG_INFO("Adding missing key '{}' to config.", it.key());
			m_json[it.key()] = it.value();
		}
	}
}

void Config::ApplyUpdates()
{
	auto removeKey = [&](const std::string_view name)
	{
		if (auto it = m_json.find(name); it != m_json.end())
			m_json.erase(it);
	};
	auto renameKey = [&](const std::string& from, const std::string& to)
	{
		json::iterator it = m_json.find(from);
		if (it != m_json.end())
		{
			std::swap(m_json[to], it.value());
			m_json.erase(it);
		}
	};
	renameKey("replays_folder", g_keyNames[ConfigKey::ReplaysDirectory]);
	renameKey("override_replays_folder", g_keyNames[ConfigKey::OverrideReplaysDirectory]);
	renameKey("game_folder", g_keyNames[ConfigKey::GameDirectory]);
	renameKey("menubar_leftside", g_keyNames[ConfigKey::MenuBarLeft]);
	removeKey("api_key");
	removeKey("use_ga");
	removeKey("save_csv");
}

std::string& Config::GetKeyName(ConfigKey key)
{
	return g_keyNames[key];
}
