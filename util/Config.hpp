// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Logger.hpp"
#include <QString>
#include <QObject>
#include <string>
#include <optional>
#include <filesystem>


namespace fs = std::filesystem;

namespace PotatoAlert {

enum StatsMode
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

	void Load();
	bool Save();

	template <typename T>
	T Get(const char* name) const
	{
		// we technically dont have to check, since we check for keys on init
		if (this->j.contains(name))
			return this->j.at(name).get<T>();
		return {};
	}

	template <typename T>
	void Set(const char* name, T value)
	{
		this->j.at(name) = value;
	}

	json j;
private:
	fs::path filePath;
	static std::optional<fs::path> GetPath(const char* fileName);
	void AddMissingKeys();
	bool CreateDefault() noexcept;
	[[nodiscard]] bool Exists() const noexcept;
signals:
#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"
	void modified();
#pragma clang diagnostic pop
};

Config& PotatoConfig();

}  // namespace PotatoAlert
