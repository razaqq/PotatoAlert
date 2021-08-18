// Copyright 2020 <github.com/razaqq>
#pragma once

#include "File.hpp"
#include "Json.hpp"

#include <QObject>
#include <QString>

#include <filesystem>
#include <optional>
#include <string>


namespace fs = std::filesystem;

namespace PotatoAlert {

enum class StatsMode
{
	Current,
	Pvp,
	Ranked,
	Clan
};

class Config : public QObject
{
	Q_OBJECT
public:
	explicit Config(std::string_view fileName);
	~Config() override;

	void Load();
	bool Save();

	template <typename T>
	T Get(const char* key) const
	{
		// we technically don't have to check, since we check for keys on init
		if (this->j.contains(key))
			return this->j.at(key).get<T>();
		return {};
	}

	template <typename T>
	void Set(const char* key, T value)
	{
		this->j.at(key) = value;
	}

	json j;
private:
	File m_file;
	fs::path m_filePath;
	static std::optional<fs::path> GetPath(std::string_view fileName);
	void AddMissingKeys();
	bool CreateDefault();
	bool CreateBackup();
	[[nodiscard]] bool Exists() const;
};

Config& PotatoConfig();

}  // namespace PotatoAlert
