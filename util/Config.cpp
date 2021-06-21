// Copyright 2020 <github.com/razaqq>

#include "Config.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include <QDir>
#include <QStandardPaths>
#include <QApplication>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <nlohmann/json.hpp>


namespace fs = std::filesystem;

using PotatoAlert::Config;
using nlohmann::json;


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
		this->j = json::parse(file);
		file.close();
		Logger::Debug("Config loaded.");
	}
	catch (json::exception& e)
	{
		Logger::Error("Cannot parse config: {}", e.what());

		if (this->CreateDefault())
			this->Load();
		else
			QApplication::exit(1);
	}

	file.close();
	Logger::Debug("Config loaded.");
}

bool Config::save()
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
	try
	{
		return fs::exists(this->filePath) && fs::is_regular_file(this->filePath);
	}
	catch (fs::filesystem_error& e)
	{
		// TODO: this is a bit wonky
		Logger::Error("Error while checking config existence: {}", e.what());
		return false;
	}
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

		fs::rename(this->filePath, backupConfig);
		this->j = g_defaultConfig;
		return this->save();
	}
	catch (fs::filesystem_error& e)
	{
		Logger::Error("Failed to create default config: {}", e.what());
		return false;
	}
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
template void Config::set(const char* name, int value);
template void Config::set(const char* name, bool value);
template void Config::set(const char* name, std::string value);
template void Config::set(const char* name, std::vector<std::string> value);

Config& PotatoAlert::PotatoConfig()
{
	static Config p("config.json");
	return p;
}
