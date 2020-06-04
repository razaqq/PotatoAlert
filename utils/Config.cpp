// Copyright 2020 <github.com/razaqq>

#include "Config.h"
#include <QDir>
#include <QStandardPaths>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

using PotatoAlert::Config;
using nlohmann::json;

Config::Config(Logger* l)
{
	this->l = l;
	this->filePath = Config::getFilePath();
	if (this->exists()) {
		this->load();
	} else {
		this->createDefault();
	}
}

Config::~Config()
{
	this->save();
}

void Config::load()
{
	this->l->Info("Loading config.");
	try {
		std::ifstream ifs(this->filePath);
		this->j = json::parse(ifs);
	}
	catch (json::exception& e)
	{
		std::string errorText = "Cannot read config: " + std::string(e.what());
		this->l->Error(errorText.c_str());
		exit(1);
	}
}

void Config::save()
{
	std::ofstream file(this->filePath);
	file << this->j;
	emit this->modified();
}

bool Config::exists() const
{
	QDir d;
	return d.exists(QString::fromStdString(this->filePath));
}

void Config::createDefault()
{
	this->l->Info("Creating default config.");
	this->j = {
		{"stats_mode", 1},  // "current mode", "pvp", "ranked", "clan"
		{"update_notifications", true},
		{"region", "eu"},
		{"api_key", "1234567890"},
		{"use_central_api", true},
		{"use_ga", true},
		{"window_height", 450},
		{"window_width", 1500},
		{"window_x", 0},
		{"window_y", 0},
		{"game_folder", ""},
		{"replays_folder", ""}
	};
	this->save();
}

template <typename T> T Config::get(char* name) const
{
	T value;
	try {
		value = this->j[name].get<T>();
	} catch (json::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	return value;
}
template int Config::get(char* name) const;
template bool Config::get(char* name) const;
template std::string Config::get(char* name) const;

template <typename T> void Config::set(char* name, T value)
{
	this->j[name] = value;
}
template void Config::set(char* name, int value);
template void Config::set(char* name, bool value);
template void Config::set(char* name, std::string value);

std::string Config::getFilePath()
{
	QString dirPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);

	QDir d;
	d.mkpath(dirPath);
	d.setPath(dirPath);

	return d.filePath("config.json").toStdString();
}
