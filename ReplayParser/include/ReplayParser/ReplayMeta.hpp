// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Json.hpp"
#include "Core/Result.hpp"
#include "Core/String.hpp"
#include "Core/Version.hpp"

#include <string>
#include <unordered_map>
#include <vector>


namespace PotatoAlert::ReplayParser {

struct ArenaInfoVehicle
{
	uint64_t ShipId;
	uint32_t Relation;
	uint32_t Id;
	std::string Name;
};

static inline Core::JsonResult<void> FromJson(const rapidjson::Value& j, ArenaInfoVehicle& v)
{
	PA_TRYA(v.ShipId, Core::FromJson<uint64_t>(j, "shipId"));
	PA_TRYA(v.Relation, Core::FromJson<uint32_t>(j, "relation"));
	PA_TRYA(v.Id, Core::FromJson<uint32_t>(j, "id"));
	PA_TRYA(v.Name, Core::FromJson<std::string>(j, "name"));
	return {};
}

struct ReplayMeta
{
	std::string MatchGroup;
	uint32_t GameMode;
	Core::Version ClientVersionFromExe;
	Core::Version ClientVersionFromXml;
	// std::unordered_map<std::string, std::vector<std::string>> WeatherParams;
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
	uint32_t BattleDuration;

	// only in version < 12.3
	std::string GameLogic;
	std::string Logic;

	// only in version >= 12.3
	std::string EventType;
	std::string GameType;
};

static Core::Version ParseClientVersion(std::string_view str)
{
	const std::vector v = Core::String::Split(str, ",");
	return Core::Version(Core::String::Join(std::span{ v }.subspan(0, 3), "."));
}

[[maybe_unused]] static Core::JsonResult<void> FromJson(const rapidjson::Value& j, ReplayMeta& m)
{
	PA_TRYA(m.MatchGroup, Core::FromJson<std::string>(j, "matchGroup"));
	PA_TRYA(m.GameMode, Core::FromJson<uint32_t>(j, "gameMode"));
	PA_TRY(clientVersionFromExe, Core::FromJson<std::string>(j, "clientVersionFromExe"));
	m.ClientVersionFromExe = ParseClientVersion(clientVersionFromExe);
	PA_TRY(clientVersionFromXml, Core::FromJson<std::string>(j, "clientVersionFromXml"));
	m.ClientVersionFromXml = ParseClientVersion(clientVersionFromXml);
	PA_TRYA(m.ScenarioUiCategoryId, Core::FromJson<uint32_t>(j, "scenarioUiCategoryId"));
	PA_TRYA(m.MapDisplayName, Core::FromJson<std::string>(j, "mapDisplayName"));
	PA_TRYA(m.MapId, Core::FromJson<uint32_t>(j, "mapId"));

	// TODO: weather params
	// Core::FromJson(j["weatherParams"], m.WeatherParams);

	PA_TRYA(m.Duration, Core::FromJson<uint32_t>(j, "duration"));
	PA_TRYA(m.Name, Core::FromJson<std::string>(j, "name"));
	PA_TRYA(m.Scenario, Core::FromJson<std::string>(j, "scenario"));
	PA_TRYA(m.PlayerId, Core::FromJson<uint32_t>(j, "playerID"));

	if (!j.HasMember("vehicles"))
		return PA_JSON_ERROR("ReplayMeta is missing key 'vehicles'");
	if (!Core::FromJson(j["vehicles"], m.Vehicles))
		return PA_JSON_ERROR("Failed to parse ReplayMeta key 'vehicles'");

	PA_TRYA(m.PlayersPerTeam, Core::FromJson<uint32_t>(j, "playersPerTeam"));
	PA_TRYA(m.DateTime, Core::FromJson<std::string>(j, "dateTime"));
	PA_TRYA(m.MapName, Core::FromJson<std::string>(j, "mapName"));
	PA_TRYA(m.PlayerName, Core::FromJson<std::string>(j, "playerName"));
	PA_TRYA(m.ScenarioConfigId, Core::FromJson<uint32_t>(j, "scenarioConfigId"));
	PA_TRYA(m.TeamsCount, Core::FromJson<uint32_t>(j, "teamsCount"));
	PA_TRYA(m.PlayerVehicle, Core::FromJson<std::string>(j, "playerVehicle"));
	PA_TRYA(m.BattleDuration, Core::FromJson<uint32_t>(j, "battleDuration"));

	if (j.HasMember("logic"))
	{
		PA_TRYA(m.Logic, Core::FromJson<std::string>(j, "logic"));
	}

	if (j.HasMember("gameLogic"))
	{
		PA_TRYA(m.GameLogic, Core::FromJson<std::string>(j, "gameLogic"));
	}

	if (j.HasMember("gameType"))
	{
		PA_TRYA(m.GameType, Core::FromJson<std::string>(j, "gameType"));
	}

	if (j.HasMember("eventType"))
	{
		PA_TRYA(m.EventType, Core::FromJson<std::string>(j, "eventType"));
	}

	return {};
}

}  // namespace PotatoAlert::ReplayParser
