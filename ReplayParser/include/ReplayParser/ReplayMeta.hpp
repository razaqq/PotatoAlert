// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Json.hpp"
#include "Core/String.hpp"
#include "Core/Version.hpp"

#include <string>
#include <unordered_map>
#include <vector>


using PotatoAlert::Core::String::Join;
using PotatoAlert::Core::String::Split;
using PotatoAlert::Core::Version;

namespace PotatoAlert::ReplayParser {

enum class Relation : uint32_t
{
	Self,
	Ally,
	Enemy,
	Neutral,
};

struct ArenaInfoVehicle
{
	uint64_t ShipId;
	uint32_t Relation;
	uint32_t Id;
	std::string Name;
};

[[maybe_unused]] static void from_json(const json& j, ArenaInfoVehicle& m)
{
	j.at("shipId").get_to(m.ShipId);
	j.at("relation").get_to(m.Relation);
	j.at("id").get_to(m.Id);
	j.at("name").get_to(m.Name);
}

struct ReplayMeta
{
	std::string MatchGroup;
	uint32_t GameMode;
	Version ClientVersionFromExe;
	Version ClientVersionFromXml;
	std::unordered_map<std::string, std::vector<std::string>> WeatherParams;
	uint32_t Duration;
	std::string Name;
	std::vector<ArenaInfoVehicle> Vehicles;
	uint32_t PlayersPerTeam;
	std::string DateTime;
	uint32_t MapId;
	std::string MapName;
	std::string MapDisplayName;
	uint32_t PlayerId;
	std::string PlayerName;
	std::string PlayerVehicle;
	std::string Scenario;
	uint32_t ScenarioConfigId;
	uint32_t ScenarioUiCategoryId;
	uint32_t TeamsCount;
	std::string EventType;
	std::string GameType;
	uint32_t BattleDuration;
};

static Version ParseClientVersion(std::string_view str)
{
	const std::vector v = Split(str, ",");
	return Version(Join(std::span{ v }.subspan(0, 3), "."));
}

[[maybe_unused]] static void from_json(const json& j, ReplayMeta& m)
{
	j.at("matchGroup").get_to(m.MatchGroup);
	j.at("gameMode").get_to(m.GameMode);
	m.ClientVersionFromExe = ParseClientVersion(j.at("clientVersionFromExe").get<std::string>());
	j.at("scenarioUiCategoryId").get_to(m.ScenarioUiCategoryId);
	j.at("mapDisplayName").get_to(m.MapDisplayName);
	j.at("mapId").get_to(m.MapId);
	m.ClientVersionFromXml = ParseClientVersion(j.at("clientVersionFromXml").get<std::string>());
	j.at("weatherParams").get_to(m.WeatherParams);
	j.at("duration").get_to(m.Duration);
	j.at("name").get_to(m.Name);
	j.at("scenario").get_to(m.Scenario);
	j.at("playerID").get_to(m.PlayerId);
	j.at("vehicles").get_to(m.Vehicles);
	j.at("playersPerTeam").get_to(m.PlayersPerTeam);
	j.at("dateTime").get_to(m.DateTime);
	j.at("mapName").get_to(m.MapName);
	j.at("playerName").get_to(m.PlayerName);
	j.at("scenarioConfigId").get_to(m.ScenarioConfigId);
	j.at("teamsCount").get_to(m.TeamsCount);
	j.at("gameType").get_to(m.GameType);
	j.at("eventType").get_to(m.EventType);
	j.at("playerVehicle").get_to(m.PlayerVehicle);
	j.at("battleDuration").get_to(m.BattleDuration);
}

}  // namespace PotatoAlert::ReplayParser
