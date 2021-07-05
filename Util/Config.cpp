// Copyright 2020 <github.com/razaqq>

#include "Config.hpp"

#include "File.hpp"
#include "Json.hpp"
#include "Log.hpp"

#include <QApplication>
#include <QDir>
#include <QStandardPaths>

#include <filesystem>
#include <iostream>
#include <string>
#include <utility>


namespace fs = std::filesystem;

using PotatoAlert::Config;


static json g_defaultConfig;

Config::Config(std::string_view fileName)
{
	g_defaultConfig = {
		{ "stats_mode", StatsMode::Pvp },
		{ "update_notifications", true },
		{ "api_key", "1234567890" },
		{ "save_csv", false },
		{ "window_height", 450 },
		{ "window_width", 1500 },
		{ "window_x", 0 },
		{ "window_y", 0 },
		{ "game_folder", "" },
		{ "override_replays_folder", false },
		{ "replays_folder", "" },
		{ "language", 0 },
		{ "menubar_leftside", true }
	};

	auto path = GetPath(fileName);
	if (!path)
	{
		LOG_ERROR("Failed to get config path.");
		QApplication::exit(1);
	}
	this->m_filePath = std::move(path.value());

	if (!this->Exists())
	{
		if (!this->CreateDefault())
		{
			LOG_ERROR("Failed to create default config.");
			QApplication::exit(1);
		}
	}
	this->Load();
	this->AddMissingKeys();
}

Config::~Config()
{
	this->Save();
	this->m_file.Close();
}

void Config::Load()
{
	LOG_TRACE("Trying to Load config...");

	this->m_file = File::Open(this->m_filePath.string(), File::Flags::Open | File::Flags::Read | File::Flags::Write);
	if (!this->m_file)
	{
		LOG_ERROR("Failed to open config file: {}", File::LastError());
		QApplication::exit(1);
	}

	std::string str;
	if (!this->m_file.ReadString(str))
	{
		LOG_ERROR("Failed to read config file: {}", File::LastError());
		QApplication::exit(1);
	}

	sax_no_exception sax(this->j);
	if (!json::sax_parse(str, &sax))
	{
		LOG_ERROR("Failed to Parse config as json.");

		this->m_file.Close();
		this->CreateBackup();

		if (this->CreateDefault())
			this->Load();
		else
			QApplication::exit(1);
	}

	LOG_TRACE("Config loaded.");
}

bool Config::Save()
{
	LOG_TRACE("Saving Config");
	if (!this->m_file)
	{
		LOG_ERROR("Cannot save config, because file is not open.");
		return false;
	}

	if (!this->m_file.Write(this->j.dump(4)))
	{
		LOG_ERROR("Failed to write config file: {}", File::LastError());
		return false;
	}
	this->m_file.FlushBuffer();
	return true;
}

bool Config::Exists() const
{
	std::error_code ec;
	bool exists = fs::exists(this->m_filePath, ec);
	if (ec)
	{
		LOG_ERROR("Error while checking config existence: {}", ec.message());
		return false;
	}

	if (exists)
	{
		bool regularFile = fs::is_regular_file(this->m_filePath, ec);
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

	this->m_file = File::Open(this->m_filePath.string(), File::Flags::Create | File::Flags::Read | File::Flags::Write);
	if (!this->m_file)
	{
		LOG_ERROR("Failed to open config file: {}", File::LastError());
		return false;
	}
	this->j = g_defaultConfig;

	bool saved = this->Save();
	this->m_file.Close();
	return saved;
}

bool Config::CreateBackup()
{
	LOG_INFO("Creating config backup");
	std::error_code ec;
	bool exists = fs::exists(this->m_filePath, ec);
	if (ec)
	{
		LOG_ERROR("Failed to check for config backup: {}", ec.message());
		return false;
	}

	if (exists)
	{
		fs::path backupConfig = this->m_filePath;
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
		fs::rename(this->m_filePath, backupConfig, ec);
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
		if (!this->j.contains(it.key()))
		{
			LOG_INFO("Adding missing key '{}' to config.", it.key());
			this->j[it.key()] = it.value();
		}
	}
}

std::optional<fs::path> Config::GetPath(std::string_view fileName)
{
	const std::string root = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).toStdString();
	const fs::path configPath = fs::path(root) / "PotatoAlert";

	std::error_code ec;

	// check if path exists
	bool exists = fs::exists(configPath, ec);
	if (ec)
	{
		LOG_ERROR("Failed to check for config path: {}", ec.message());
		return {};
	}

	if (!exists)
	{
		fs::create_directories(configPath, ec);
		if (ec)
		{
			LOG_ERROR("Failed to create dirs for config path: {}", ec.message());
			return {};
		}
	}

	return (configPath / fileName);
}

Config& PotatoAlert::PotatoConfig()
{
	static Config p("config.json");
	return p;
}
