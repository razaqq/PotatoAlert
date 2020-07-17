// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Logger.h"
#include <QString>
#include <QObject>
#include <string>
#include <nlohmann/json.hpp>


namespace PotatoAlert {

class Config : public QObject
{
	Q_OBJECT
public:
	explicit Config();
	~Config() override;

	void load();
	void save();
	void createDefault();
	[[nodiscard]] bool exists() const;

	template <typename T> T get(const char* name) const;
	template <typename T> void set(const char* name, T value);

	nlohmann::json j;
private:
	std::string filePath;
	static std::string getFilePath() ;
signals:
	void modified();
};

Config& PotatoConfig();

}  // namespace PotatoAlert