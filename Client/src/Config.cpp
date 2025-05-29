// Copyright 2025 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/Game.hpp"

#include "Core/Directory.hpp"
#include "Core/File.hpp"
#include "Core/Format.hpp"
#include "Core/Json.hpp"


using PotatoAlert::Client::Config;
using PotatoAlert::Client::ConfigManager;
using PotatoAlert::Client::ConfigResult;
using PotatoAlert::Client::StatsMode;
using PotatoAlert::Client::TableLayout;
using PotatoAlert::Client::TeamStatsMode;
using PotatoAlert::Core::File;
using PotatoAlert::Core::JsonResult;
using PotatoAlert::Core::Result;
namespace fs = std::filesystem;

namespace {

enum class ConfigError
{
	FailedToOpenFile,

};

}  // namespace

template<>
struct glz::meta<Config>
{
	using T = Config;
	static constexpr auto value = object
	(
		"version", &T::Version,
		"stats_mode", &T::StatsMode,
		"team_damage_mode", &T::TeamDamageMode,
		"team_win_rate_mode", &T::TeamWinRateMode,
		"table_layout", &T::TableLayout,
		"update_notifications", &T::UpdateNotifications,
		"minimize_tray", &T::MinimizeTray,
		"match_history", &T::MatchHistory,
		"save_match_csv", &T::SaveMatchCsv,
		"window_height", &T::WindowHeight,
		"window_width", &T::WindowWidth,
		"window_x", &T::WindowX,
		"window_y", &T::WindowY,
		"window_state", &T::WindowState,
		"game_directories", &T::GameDirectories,
		"language", &T::Language,
		"menubar_left", &T::MenuBarLeft,
		"show_karma", &T::ShowKarma,
		"font_shadow", &T::FontShadow,
		"anonymize_player_names_screenshots", &T::AnonymizePlayers,
		"font", &T::Font,
		"font_scaling", &T::FontScaling,
		"allow_sending_usage_stats", &T::AllowSendingUsageStats
	);
};

template<>
struct glz::meta<StatsMode>
{
	using enum StatsMode;
	static constexpr auto value = enumerate
	(
		"current", Current,
		"pvp", Pvp,
		"ranked", Ranked,
		"cooperative", Cooperative
	);
};

template<>
struct glz::meta<TeamStatsMode>
{
	using enum TeamStatsMode;
	static constexpr auto value = enumerate
	(
		"weighted", Weighted,
		"average", Average,
		"median", Median
	);
};

template<>
struct glz::meta<TableLayout>
{
	using enum TableLayout;
	static constexpr auto value = enumerate
	(
		"horizontal", Horizontal,
		"vertical", Vertical
	);
};

ConfigManager::ConfigManager(std::filesystem::path path) : m_config(Config{}), m_path(std::move(path))
{

}

ConfigResult<void> ConfigManager::Init()
{
	const Result<bool> exists = Core::PathExists(m_path);
	if (!exists)
	{
		return PA_ERROR(fmt::format("Failed to check if config file exists: {}", exists.error().message()));
	}
	if (exists.value())
	{
		PA_TRYV(Load());
		PA_TRYV(Migrate());
		PA_TRY(valid, IsValid());
		if (!valid)
		{
			const Result<void> backupRes = CreateBackup();
			if (!backupRes)
			{
				return PA_ERROR(fmt::format("Failed to create config backup: {}", backupRes.error().message()));
			}
			const Result<Config> def = GetDefault();
			if (!def)
			{
				return PA_ERROR(fmt::format("Failed get default config: {}", def.error().message()));
			}
			m_config = *def;
			PA_TRYV(Save());
		}
	}
	else
	{
		const Result<Config> def = GetDefault();
		if (!def)
		{
			return PA_ERROR(fmt::format("Failed get default config: {}", def.error().message()));
		}
		m_config = *def;
		PA_TRYV(Save());
	}

	return {};
}

Result<Config> ConfigManager::GetDefault()
{
	PA_TRY(gameDirectories, Game::GetDefaultGamePaths());

	return Config
	{
		.Version = 0,

		.StatsMode = StatsMode::Pvp,
		.TeamDamageMode = TeamStatsMode::Average,
		.TeamWinRateMode = TeamStatsMode::Average,
		.TableLayout = TableLayout::Horizontal,
		.UpdateNotifications = true,
		.AllowSendingUsageStats = true,
		.MinimizeTray = false,
		.MatchHistory = true,
		.SaveMatchCsv = false,
		.MenuBarLeft = true,
		.ShowKarma = false,
		.AnonymizePlayers = false,
		.WindowHeight = 450,
		.WindowWidth = 1500,
		.WindowX = 0,
		.WindowY = 0,
		.Language = 0,
		.WindowState = 0x00000008,  // TODO: Qt::WindowState::WindowActive
		.Font = "Roboto",
		.FontShadow = true,
		.FontScaling = 100,
		.GameDirectories = { gameDirectories.begin(), gameDirectories.end() },
	};
}

ConfigResult<void> ConfigManager::Load()
{
	File file = File::Open(m_path, File::Flags::Open | File::Flags::Read);
	if (!file)
	{
		return PA_ERROR(File::LastError());
	}

	std::string json;
	if (!file.ReadAllString(json))
	{
		return PA_ERROR(File::LastError());
	}

	PA_TRYA(m_config, Core::Json::Read<Config>(json));

	return {};
}

JsonResult<void> ConfigManager::Save() const
{
	const File file = File::Open(m_path, File::Flags::Create | File::Flags::Open | File::Flags::Write);
	if (!file)
	{
		return PA_ERROR(File::LastError());
	}

	PA_TRY(str, Core::Json::Write(m_config));
	if (!file.WriteString(str))
	{
		return PA_ERROR(File::LastError());
	}

	return {};
}

ConfigResult<bool> ConfigManager::IsValid() const
{
	return true;
}

ConfigResult<void> ConfigManager::Migrate()
{
	if (m_config.Version > 0)
	{
		
	}
	return {};
}

Result<void> ConfigManager::CreateBackup() const
{
	PA_TRY(exists, Core::PathExists(m_path));
	if (!exists)
	{
		return {};
	}

	fs::path backup = m_path;
	backup.replace_extension(".bak");

	PA_TRY(backupExists, Core::PathExists(backup));
	if (backupExists)
	{
		std::error_code ec;
		fs::remove(backup, ec);
		if (ec)
		{
			return PA_ERROR(ec);
		}
	}

	std::error_code ec;
	fs::rename(m_path, backup, ec);
	if (ec)
	{
		return PA_ERROR(ec);
	}
	
	return {};
}

