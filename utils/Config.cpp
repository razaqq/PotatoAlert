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
	if (this->exists())
	{
		this->load();
		this->addMissingKeys();
	}
	else
	{
		this->createDefault();
	}
}

Config::~Config()
{
	this->save();
}

void Config::load()
{
	Logger::Debug("Trying to load config.");
	try
	{
		std::ifstream ifs(this->filePath);
		this->j = json::parse(ifs);
        Logger::Debug("Config loaded.");
	}
	catch (json::exception& e)
	{
		Logger::Error("Cannot read config: {}", e.what());
		try
		{
			std::string backupConfig = this->filePath.append(".bak");
			if (fs::exists(backupConfig))
				fs::remove(backupConfig);
			fs::rename(this->filePath, backupConfig);
			this->createDefault();
		}
		catch (fs::filesystem_error& e)
		{
			Logger::Error(e.what());
			QApplication::exit(1);
		}
	}
}

void Config::save()
{
	std::ofstream file(this->filePath);
	file << this->j;
}

bool Config::exists() const
{
	QDir d;
	return d.exists(QString::fromStdString(this->filePath));
}

void Config::createDefault()
{
	Logger::Info("Creating default config.");
	this->j = g_defaultConfig;
	this->save();
}

void Config::addMissingKeys()
{
	for (auto it = g_defaultConfig.begin(); it != g_defaultConfig.end(); ++it)
	{
		if (!this->j.contains(it.key()))
		{
			Logger::Debug("Adding missing key {} to config.", it.key());
			this->j[it.key()] = it.value();
		}
	}
}

template <typename T> T Config::get(const char* name) const
{
	T value;
	try
	{
		value = this->j[name].get<T>();
	}
	catch (json::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
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

std::string Config::getFilePath(const char* fileName)
{
	QString dirPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/PotatoAlert";

	QDir d;
	d.mkpath(dirPath);
	d.setPath(dirPath);

	return d.filePath(fileName).toStdString();
}

Config& PotatoAlert::PotatoConfig()
{
	static Config p("config.json");
	return p;
}
