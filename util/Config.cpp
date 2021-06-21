// Copyright 2020 <github.com/razaqq>

#include "Config.hpp"
#include "Logger.hpp"
#include "Json.hpp"
#include <QDir>
#include <QStandardPaths>
#include <QApplication>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>


namespace fs = std::filesystem;

using PotatoAlert::Config;


static json g_defaultConfig;

Config::Config(const char* fileName)
{
	g_defaultConfig = {
			{"stats_mode", pvp},  // "current mode", "pvp", "ranked", "clan"
			{"update_notifications", true},
			{"api_key", "1234567890"},
			{"save_csv", false},
			{"use_ga", true},
			{"window_height", 450},
			{"window_width", 1500},
			{"window_x", 0},
			{"window_y", 0},
			{"game_folder", ""},
			{"override_replays_folder", false},
			{"replays_folder", ""},
			{"language", 0},
			{"menubar_leftside", true}
	};

	auto path = Config::GetPath(fileName);
	if (path.has_value())
	{
		this->filePath = path.value();
	}
	else
	{
		Logger::Error("Failed to get config path.");
		QApplication::exit(1);
	}

	this->Load();
	this->AddMissingKeys();
}

Config::~Config()
{
	this->Save();
}

void Config::Load()
{
	Logger::Debug("Trying to Load config...");

	if (!this->Exists())
	{
		if (!this->CreateDefault())
		{
			Logger::Error("Failed to create default config.");
			QApplication::exit(1);
		}
	}

	std::ifstream file(this->filePath);
	if (!file.is_open())
	{
		Logger::Error("Failed to open config file for reading.");
		QApplication::exit(1);
	}

	sax_no_exception sax(this->j);
	if (!json::sax_parse(file, &sax))
	{
		Logger::Error("Failed to Parse config as json.");

		if (this->CreateDefault())
			this->Load();
		else
			QApplication::exit(1);
	}

	file.close();
	Logger::Debug("Config loaded.");
}

bool Config::Save()
{
	std::ofstream file(this->filePath);
	if (!file.is_open())
	{
		Logger::Error("Failed to open config file for writing.");
		return false;
	}
	file << this->j;
	file.close();
	return true;
}

bool Config::Exists() const noexcept
{
	std::error_code ec;
	bool exists = fs::exists(this->filePath, ec);
	if (ec)
	{
		Logger::Error("Error while checking config existence: {}", ec.message());
		return false;
	}
	bool regularFile = fs::is_regular_file(this->filePath, ec);
	if (ec)
	{
		Logger::Error("Error while checking config existence: {}", ec.message());
		return false;
	}
	return exists && regularFile;
}

bool Config::CreateDefault() noexcept
{
	Logger::Info("Creating new default config.");
	if (!this->Exists())
	{
		this->j = g_defaultConfig;
		return this->Save();
	}

	Logger::Info("Creating backup of old config.");
	fs::path backupConfig(this->filePath.string() + ".bak");  // TODO

	std::error_code ec;

	// check if backup exists
	bool exists = fs::exists(backupConfig, ec);
	if (ec)
	{
		Logger::Error("Failed to check for config backup: {}", ec.message());
		return false;
	}

	// remove backup if it exists
	if (!exists)
	{
		fs::remove(backupConfig, ec);
		if (ec)
		{
			Logger::Error("Failed to remove config backup: {}", ec.message());
			return false;
		}
	}

	// create backup
	fs::rename(this->filePath, backupConfig, ec);
	if (ec)
	{
		Logger::Error("Failed to create config backup: {}", ec.message());
		return false;
	}

	this->j = g_defaultConfig;
	return this->Save();
}

void Config::AddMissingKeys()
{
	for (auto it = g_defaultConfig.begin(); it != g_defaultConfig.end(); ++it)
	{
		if (!this->j.contains(it.key()))
		{
			Logger::Info("Adding missing key '{}' to config.", it.key());
			this->j[it.key()] = it.value();
		}
	}
}

std::optional<fs::path> Config::GetPath(const char* fileName)
{
	const std::string root = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).toStdString();
	const fs::path configPath = fs::path(root) / "PotatoAlert";

	std::error_code ec;

	// check if path exists
	bool exists = fs::exists(configPath, ec);
	if (ec)
	{
		Logger::Error("Failed to check for config path: {}", ec.message());
		return {};
	}

	if (!exists)
	{
		fs::create_directories(configPath, ec);
		if (ec)
		{
			Logger::Error("Failed to create dirs for config path: {}", ec.message());
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
