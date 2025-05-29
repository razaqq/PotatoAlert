// Copyright 2025 <github.com/razaqq>
#pragma once

#include "Core/Json.hpp"
#include "Core/Result.hpp"

#include <cstdint>
#include <filesystem>
#include <unordered_set>


namespace PotatoAlert::Client {

enum class StatsMode
{
	Current,
	Pvp,
	Ranked,
	Cooperative,
};

enum class TeamStatsMode
{
	Weighted,
	Average,
	Median,
};

enum class TableLayout
{
	Horizontal,
	Vertical,
};

static constexpr int32_t CurrentVersion = 0;

struct Config
{
	int32_t Version = CurrentVersion;

	StatsMode StatsMode;
	TeamStatsMode TeamDamageMode;
	TeamStatsMode TeamWinRateMode;
	TableLayout TableLayout;
	bool UpdateNotifications;
	bool AllowSendingUsageStats;
	bool MinimizeTray;
	bool MatchHistory;
	bool SaveMatchCsv;
	bool MenuBarLeft;
	bool ShowKarma;
	bool AnonymizePlayers;

	int32_t WindowHeight;
	int32_t WindowWidth;
	int32_t WindowX;
	int32_t WindowY;
	int32_t Language;
	int32_t WindowState;

	std::string Font;
	bool FontShadow;
	int32_t FontScaling;

	std::unordered_set<std::filesystem::path> GameDirectories;
};

template<typename T>
using ConfigResult = std::expected<T, std::string>;

class ConfigManager
{
public:
	explicit ConfigManager(std::filesystem::path path);

	ConfigResult<void> Init();
	ConfigResult<void> Save() const;
	ConfigResult<void> Load();

	[[nodiscard]] Config& GetConfig()
	{
		return m_config;
	}

private:
	static Core::Result<Config> GetDefault();
	ConfigResult<bool> IsValid() const;
	ConfigResult<void> Migrate();
	Core::Result<void> CreateBackup() const;

private:
	Config m_config;
	std::filesystem::path m_path;
};

}  // namespace PotatoAlert::Client
