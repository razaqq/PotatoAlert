// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/ServiceProvider.hpp"

#include "Core/File.hpp"
#include "Core/Json.hpp"

#include <QObject>
#include <QString>

#include <filesystem>
#include <string>


namespace fs = std::filesystem;
using PotatoAlert::Core::File;

namespace PotatoAlert::Client {

enum class ConfigType : uint8_t
{
	String        = 0,
	Bool          = 8,
	Int           = 16,
	Float         = 24,
	StatsMode     = 36,
	TeamStatsMode = 48,
};

#define DECL_TYPE(name, type, value) name = static_cast<uint64_t>((value)) << static_cast<uint8_t>((type))

enum class ConfigKey : uint64_t
{
	DECL_TYPE(StatsMode,                ConfigType::StatsMode,     1),
	DECL_TYPE(TeamDamageMode,           ConfigType::TeamStatsMode, 1),
	DECL_TYPE(TeamWinRateMode,          ConfigType::TeamStatsMode, 2),
	DECL_TYPE(MinimizeTray,             ConfigType::Bool,          1),
	DECL_TYPE(MatchHistory,             ConfigType::Bool,          2),
	DECL_TYPE(MenuBarLeft,              ConfigType::Bool,          3),
	DECL_TYPE(UpdateNotifications,      ConfigType::Bool,          4),
	DECL_TYPE(OverrideReplaysDirectory, ConfigType::Bool,          5),
	DECL_TYPE(SaveMatchCsv,             ConfigType::Bool,          6),
	DECL_TYPE(WindowHeight,             ConfigType::Int,           1),
	DECL_TYPE(WindowWidth,              ConfigType::Int,           2),
	DECL_TYPE(WindowX,                  ConfigType::Int,           3),
	DECL_TYPE(WindowY,                  ConfigType::Int,           4),
	DECL_TYPE(Language,                 ConfigType::Int,           5),
	DECL_TYPE(WindowState,              ConfigType::Int,           6),
	DECL_TYPE(GameDirectory,            ConfigType::String,        1),
	DECL_TYPE(ReplaysDirectory,         ConfigType::String,        2),
};

static constexpr bool IsType(ConfigKey key, ConfigType type)
{
	return static_cast<uint64_t>(key) & (static_cast<uint64_t>(0xFF) << static_cast<uint8_t>(type));
}

enum class StatsMode
{
	Current,
	Pvp,
	Ranked,
	Pve,
};

NLOHMANN_JSON_SERIALIZE_ENUM(StatsMode,
{
	{ StatsMode::Current, "current" },
	{ StatsMode::Pvp,     "pvp"     },
	{ StatsMode::Ranked,  "ranked"  },
	{ StatsMode::Pve,     "pve"     },
})

enum class TeamStatsMode
{
	Weighted,
	Average,
	Median,
};

NLOHMANN_JSON_SERIALIZE_ENUM(TeamStatsMode,
{
	{ TeamStatsMode::Weighted, "weighted" },
	{ TeamStatsMode::Average,  "average"  },
	{ TeamStatsMode::Median,   "median"   },
})

class Config
{
public:
	explicit Config(const std::string& filePath);
	Config(const Config& config) = delete;
	Config(Config&& config) noexcept = delete;
	Config& operator=(const Config& config) = delete;
	Config operator=(Config&& config) noexcept = delete;

	~Config();

	void Load();
	bool Save() const;

private:
	template <ConfigKey Key, typename T>
	[[nodiscard]] T BaseGet() const
	{
		// we technically don't have to check, since we check for keys on init
		if (m_json.contains(GetKeyName(Key)))
			return m_json.at(GetKeyName(Key)).get<T>();
		return {};
	}

	template<ConfigKey Key, typename T>
	void BaseSet(T value)
	{
		m_json[GetKeyName(Key)] = value;
	}

public:
	template<ConfigKey Key> requires(IsType(Key, ConfigType::StatsMode))
	[[nodiscard]] StatsMode Get() const { return BaseGet<Key, StatsMode>(); }

	template<ConfigKey Key>
	requires(IsType(Key, ConfigType::TeamStatsMode))
	[[nodiscard]] TeamStatsMode Get() const { return BaseGet<Key, TeamStatsMode>(); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::String))
	[[nodiscard]] std::string Get() const { return BaseGet<Key, std::string>(); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::Bool))
	[[nodiscard]] bool Get() const { return BaseGet<Key, bool>(); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::Int))
	[[nodiscard]] int Get() const { return BaseGet<Key, int>(); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::StatsMode))
	void Set(StatsMode value) { BaseSet<Key>(value); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::TeamStatsMode))
	void Set(TeamStatsMode value) { BaseSet<Key>(value); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::String))
	void Set(std::string_view value) { BaseSet<Key>(value); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::Bool))
	void Set(bool value) { BaseSet<Key>(value); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::Int))
	void Set(int value) { BaseSet<Key>(value); }


private:
	json m_json;
	File m_file;
	std::string m_filePath;
	void AddMissingKeys();
	void ApplyUpdates();
	bool CreateDefault();
	[[nodiscard]] static std::string& GetKeyName(ConfigKey key);
	bool CreateBackup() const;
	[[nodiscard]] bool Exists() const;
};

}  // namespace PotatoAlert::Client
