// Copyright 2020 <github.com/razaqq>

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
			{"language", 0}
	};
	this->filePath = Config::getFilePath(fileName);
	this->load();
	this->addMissingKeys();
}

Config::~Config()
{
	this->save();
}

void Config::load()
{
	Logger::Debug("Trying to load config...");
	try
	{
		if (!this->exists())
		{
			if (!this->createDefault())
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

		if (this->createDefault())
			this->load();
		else
			QApplication::exit(1);
	}
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

bool Config::exists() const
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

bool Config::createDefault()
{
	try
	{
		Logger::Info("Creating new default config.");
		if (!this->exists())
		{
			this->j = g_defaultConfig;
			return this->save();
		}

		Logger::Info("Creating backup of old config.");
		fs::path backupConfig(this->filePath.string() + ".bak");  // TODO
		if (fs::exists(backupConfig))
			fs::remove(backupConfig);

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

void Config::addMissingKeys()
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

fs::path Config::getFilePath(const char* fileName)
{
	const std::string root = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).toStdString();
	const fs::path configPath = fs::path(root) / "PotatoAlert";

	try
	{
		if (!fs::exists(configPath))
			fs::create_directories(configPath);
	}
	catch (fs::filesystem_error& e)
	{
		Logger::Error("Can't create config dir: {}", e.what());
	}

	return (configPath / fileName);
}

template <typename T>
T Config::get(const char* name) const
{
	try
	{
		return this->j[name].get<T>();
	}
	catch (json::exception& e)
	{
		Logger::Error("Can't get key '{}' from config: {}", name, e.what());
	}
	T value;
	return value;
}
template int Config::get(const char* name) const;
template bool Config::get(const char* name) const;
template std::string Config::get(const char* name) const;
template std::vector<std::string> Config::get(const char* name) const;

template <typename T> void Config::set(const char* name, T value)
{
	this->j[name] = value;
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
