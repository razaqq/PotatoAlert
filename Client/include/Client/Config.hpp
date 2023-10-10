// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/ServiceProvider.hpp"

#include "Core/Encoding.hpp"
#include "Core/File.hpp"
#include "Core/Json.hpp"
#include "Core/Result.hpp"

#include <cstdint>
#include <filesystem>
#include <string>


namespace fs = std::filesystem;
using PotatoAlert::Core::File;
using PotatoAlert::Core::Result;

namespace PotatoAlert::Client {

enum class ConfigType : uint8_t
{
	String        = 8 * 0,
	Bool          = 8 * 1,
	Int           = 8 * 2,
	Float         = 8 * 3,
	StatsMode     = 8 * 4,
	TeamStatsMode = 8 * 5,
	TableLayout   = 8 * 6,
	Path          = 8 * 7,
};

#define DECL_TYPE(name, type, value) name = static_cast<uint64_t>((value)) << static_cast<uint8_t>((type))

enum class ConfigKey : uint64_t
{
	DECL_TYPE(StatsMode,                ConfigType::StatsMode,     1),
	DECL_TYPE(TeamDamageMode,           ConfigType::TeamStatsMode, 1),
	DECL_TYPE(TeamWinRateMode,          ConfigType::TeamStatsMode, 2),
	DECL_TYPE(TableLayout,              ConfigType::TableLayout,   1),
	DECL_TYPE(MinimizeTray,             ConfigType::Bool,          1),
	DECL_TYPE(MatchHistory,             ConfigType::Bool,          2),
	DECL_TYPE(MenuBarLeft,              ConfigType::Bool,          3),
	DECL_TYPE(UpdateNotifications,      ConfigType::Bool,          4),
	DECL_TYPE(ShowKarma,                ConfigType::Bool,          5),
	DECL_TYPE(SaveMatchCsv,             ConfigType::Bool,          6),
	DECL_TYPE(WindowHeight,             ConfigType::Int,           1),
	DECL_TYPE(WindowWidth,              ConfigType::Int,           2),
	DECL_TYPE(WindowX,                  ConfigType::Int,           3),
	DECL_TYPE(WindowY,                  ConfigType::Int,           4),
	DECL_TYPE(Language,                 ConfigType::Int,           5),
	DECL_TYPE(WindowState,              ConfigType::Int,           6),
	DECL_TYPE(GameDirectory,            ConfigType::Path,          1),
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
	Cooperative,
};
PA_JSON_SERIALIZE_ENUM(StatsMode,
{
	{ StatsMode::Current,     "current"     },
	{ StatsMode::Pvp,         "pvp"         },
	{ StatsMode::Ranked,      "ranked"      },
	{ StatsMode::Cooperative, "cooperative" },
})

enum class TeamStatsMode
{
	Weighted,
	Average,
	Median,
};
PA_JSON_SERIALIZE_ENUM(TeamStatsMode,
{
	{ TeamStatsMode::Weighted, "weighted" },
	{ TeamStatsMode::Average,  "average"  },
	{ TeamStatsMode::Median,   "median"   },
})

enum class TableLayout
{
	Horizontal,
	Vertical,
};
PA_JSON_SERIALIZE_ENUM(TableLayout,
{
	{ TableLayout::Horizontal, "horizontal" },
	{ TableLayout::Vertical,   "vertical"   },
})

class Config
{
public:
	explicit Config(const fs::path& filePath);
	Config(const Config& config) = delete;
	Config(Config&& config) noexcept = delete;
	Config& operator=(const Config& config) = delete;
	Config operator=(Config&& config) noexcept = delete;

	~Config();

	void Load();
	bool Save() const;

private:
	template<ConfigKey Key, typename T> requires(IsType(Key, ConfigType::Path) && std::is_same_v<T, std::filesystem::path>)
	[[nodiscard]] std::filesystem::path BaseGet() const
	{
		if (m_json.HasMember(GetKeyName(Key).data()))
		{
			const std::string raw = Core::FromJson<std::string>(m_json[GetKeyName(Key).data()]);
			if (const Result<std::filesystem::path> res = Core::Utf8ToPath(raw))
				return res.value();
		}
		return {};
	}

	template <ConfigKey Key, typename T>
	[[nodiscard]] T BaseGet() const
	{
		// we technically don't have to check, since we check for keys on init
		if (m_json.HasMember(GetKeyName(Key).data()))
		{
			if constexpr (std::is_enum_v<T>)
			{
				T t;
				FromJson(m_json[GetKeyName(Key).data()], t);
				return t;
			}
			else
			{
				return Core::FromJson<T>(m_json[GetKeyName(Key).data()]);
			}
		}
		return {};
	}

	template<ConfigKey Key> requires(IsType(Key, ConfigType::Path))
	void BaseSet(const std::filesystem::path& value)
	{
		if (const Result<std::string> res = Core::PathToUtf8(value))
		{
			m_json[GetKeyName(Key).data()].SetString(res.value(), m_json.GetAllocator());
		}
	}

	template<ConfigKey Key, typename T>
	void BaseSet(T value)
	{
		if constexpr (std::is_enum_v<T>)
		{
			m_json[GetKeyName(Key).data()] = ToJson(value);
		}
		else
		{
			m_json[GetKeyName(Key).data()] = Core::ToJson(value);
		}
	}

public:
	template<ConfigKey Key> requires(IsType(Key, ConfigType::StatsMode))
	[[nodiscard]] StatsMode Get() const { return BaseGet<Key, StatsMode>(); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::TeamStatsMode))
	[[nodiscard]] TeamStatsMode Get() const { return BaseGet<Key, TeamStatsMode>(); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::TableLayout))
	[[nodiscard]] TableLayout Get() const { return BaseGet<Key, TableLayout>(); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::String))
	[[nodiscard]] std::string Get() const { return BaseGet<Key, std::string>(); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::Bool))
	[[nodiscard]] bool Get() const { return BaseGet<Key, bool>(); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::Int))
	[[nodiscard]] int Get() const { return BaseGet<Key, int>(); }
	
	template<ConfigKey Key> requires(IsType(Key, ConfigType::Path))
	[[nodiscard]] std::filesystem::path Get() const { return BaseGet<Key, std::filesystem::path>(); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::StatsMode))
	void Set(StatsMode value) { BaseSet<Key>(value); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::TeamStatsMode))
	void Set(TeamStatsMode value) { BaseSet<Key>(value); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::TableLayout))
	void Set(TableLayout value) { BaseSet<Key>(value); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::String))
	void Set(std::string_view value) { BaseSet<Key>(value); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::Bool))
	void Set(bool value) { BaseSet<Key>(value); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::Int))
	void Set(int value) { BaseSet<Key>(value); }

	template<ConfigKey Key> requires(IsType(Key, ConfigType::Path))
	void Set(const std::filesystem::path& value) { BaseSet<Key>(value); }


private:
	rapidjson::Document m_json;
	File m_file;
	fs::path m_filePath;
	void Validate();
	void SetDefault(ConfigKey key);
	void ApplyUpdates();
	bool CreateDefault();
	[[nodiscard]] static std::string_view GetKeyName(ConfigKey key);
	bool CreateBackup() const;
	[[nodiscard]] bool Exists() const;
};

}  // namespace PotatoAlert::Client
