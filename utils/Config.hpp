// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Logger.hpp"
#include <QString>
#include <QObject>
#include <string>
#include <filesystem>
#include <nlohmann/json.hpp>


namespace fs = std::filesystem;

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
	bool save();

	[[nodiscard]] bool exists() const;

	template <typename T> T get(const char* name) const;
	template <typename T> void set(const char* name, T value);

	nlohmann::json j;
private:
	fs::path filePath;
	static fs::path getFilePath(const char* fileName);
	void addMissingKeys();
	bool createDefault();
signals:
#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"
	void modified();
#pragma clang diagnostic pop
};

Config& PotatoConfig();

}  // namespace PotatoAlert
