// Copyright 2020 <github.com/razaqq>

#include "Logger.h"
#include "Config.h"
#include <QDir>
#include <QStandardPaths>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>


using PotatoAlert::Config;
using nlohmann::json;

// TODO: create default config if config cannot be parsed

Config::Config()
{
	this->filePath = Config::getFilePath();
	if (this->exists())
		this->load();
	else
		this->createDefault();
}

Config::~Config()
{
	this->save();
}

void Config::load()
{
    Logger::Debug("Loading config.");
	try {
		std::ifstream ifs(this->filePath);
		this->j = json::parse(ifs);
	}
	catch (json::exception& e)
	{
		std::string errorText = "Cannot read config: " + std::string(e.what());
        PotatoLogger().Error(errorText.c_str());
		exit(1);
	}
}

void Config::save()
{
	std::ofstream file(this->filePath);
	file << this->j;
	// emit this->modified();
}

bool Config::exists() const
{
	QDir d;
	return d.exists(QString::fromStdString(this->filePath));
}

void Config::createDefault()
{
    PotatoLogger().Info("Creating default config.");
	this->j = {
		{"stats_mode", 1},  // "current mode", "pvp", "ranked", "clan"
		{"update_notifications", true},
		{"region", "eu"},
		{"api_key", "1234567890"},
		{"save_csv", false},
		{"use_ga", true},
		{"window_height", 450},
		{"window_width", 1500},
		{"window_x", 0},
		{"window_y", 0},
		{"game_folder", ""},
        {"language", 0}
	};
	this->save();
}

template <typename T> T Config::get(const char* name) const
{
	T value;
	try {
		value = this->j[name].get<T>();
	} catch (json::exception& e) {
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

std::string Config::getFilePath()
{
	QString dirPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/PotatoAlert";

	QDir d;
	d.mkpath(dirPath);
	d.setPath(dirPath);

	return d.filePath("config.json").toStdString();
}

Config& PotatoAlert::PotatoConfig()
{
    static Config p;
    return p;
}
