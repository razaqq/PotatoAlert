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

struct VehicleInfoMeta
{
	uint64_t ShipId;
	uint32_t Relation;
	uint32_t Id;
	std::string Name;
};

[[maybe_unused]] static void from_json(const json& j, VehicleInfoMeta& m)
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
	uint32_t ScenarioUiCategoryId;
	std::string MapDisplayName;
	uint32_t MapId;
	Version ClientVersionFromXml;
	std::unordered_map<std::string, std::vector<std::string>> WeatherParams;
	//mapBorder null;
	uint32_t Duration;
	std::string GameLogic;
	std::string Name;
	std::string Scenario;
	uint32_t PlayerId;
	std::vector<VehicleInfoMeta> Vehicles;
	uint32_t PlayersPerTeam;
	std::string DateTime;
	std::string MapName;
	std::string PlayerName;
	uint32_t ScenarioConfigId;
	uint32_t TeamsCount;
	std::string Logic;
	std::string PlayerVehicle;
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
	j.at("gameLogic").get_to(m.GameLogic);
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
	j.at("logic").get_to(m.Logic);
	j.at("playerVehicle").get_to(m.PlayerVehicle);
	j.at("battleDuration").get_to(m.BattleDuration);
}

}  // namespace PotatoAlert::ReplayParser
