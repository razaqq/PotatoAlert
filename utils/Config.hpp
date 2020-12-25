// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Logger.hpp"
#include <QString>
#include <QObject>
#include <string>
#include <nlohmann/json.hpp>


namespace PotatoAlert {

enum statsMode
{
	current,
	pvp,
	ranked,
	clan
};

class Config : public QObject
{
	Q_OBJECT
public:
	explicit Config(const char* fileName);
	~Config() override;

	void load();
	void save();
	void createDefault();
	void addMissingKeys();
	[[nodiscard]] bool exists() const;

	template <typename T> T get(const char* name) const;
	template <typename T> void set(const char* name, T value);

	nlohmann::json j;
private:
	std::string filePath;
	static std::string getFilePath(const char* fileName);
signals:
	void modified();
};

Config& PotatoConfig();

}  // namespace PotatoAlert
