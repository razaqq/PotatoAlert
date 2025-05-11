// Copyright 2025 <github.com/razaqq>

#include "Core/Json.hpp"
#include "Core/String.hpp"
#include "Core/Version.hpp"

#include "ReplayParser/ReplayMeta.hpp"

#include <string>
#include <vector>


using PotatoAlert::Core::Version;
using PotatoAlert::Core::String::Join;
using PotatoAlert::Core::String::Split;
using PotatoAlert::ReplayParser::ArenaInfoVehicle;
using PotatoAlert::ReplayParser::ReplayMeta;

template<>
struct glz::meta<ArenaInfoVehicle>
{
	using T = ArenaInfoVehicle;
	static constexpr auto value = glz::object
	(
		"shipId", &T::ShipId,
		"relation", &T::Relation,
		"id", &T::Id,
		"name", &T::Name
	);
};

template<>
struct glz::meta<ReplayMeta>
{
	using T = ReplayMeta;
	static constexpr auto value = glz::object
	(
		"matchGroup", &T::MatchGroup,
		"gameMode", &T::GameMode,
		"clientVersionFromExe", &T::ClientVersionFromExe,
		"clientVersionFromXml", &T::ClientVersionFromXml,
		// "weatherParams", &T::WeatherParams,
		"duration", &T::Duration,
		"name", &T::Name,
		"vehicles", &T::Vehicles,
		"playersPerTeam", &T::PlayersPerTeam,
		"dateTime", &T::DateTime,
		"mapId", &T::MapId,
		"mapName", &T::MapName,
		"mapDisplayName", &T::MapDisplayName,
		"playerID", &T::PlayerId,
		"playerName", &T::PlayerName,
		"playerVehicle", &T::PlayerVehicle,
		"scenario", &T::Scenario,
		"scenarioConfigId", &T::ScenarioConfigId,
		"scenarioUiCategoryId", &T::ScenarioUiCategoryId,
		"teamsCount", &T::TeamsCount,
		"battleDuration", &T::BattleDuration
	);
};

namespace glz {

template<>
struct from<JSON, Version>
{
	template<auto Opts>
	static void op(Version& version, auto&&... args)
	{
		std::string_view str = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
		parse<JSON>::op<Opts>(str, args...);

		const std::vector v = Split(str, ",");
		version = Version(Join(std::span{ v }.subspan(0, 3), "."));
	}
};

}  // namespace glz

PotatoAlert::Core::JsonResult<ReplayMeta> PotatoAlert::ReplayParser::ParseMeta(std::string_view str)
{
	ReplayMeta meta;
	constexpr glz::opts opts{ .error_on_unknown_keys = false };
	const auto ec = glz::read<opts>(meta, str);
	if (ec)
	{
		return std::unexpected(glz::format_error(ec, str));
	}
	return meta;
}

