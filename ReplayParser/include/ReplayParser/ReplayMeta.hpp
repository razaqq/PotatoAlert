// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Json.hpp"
#include "Core/Result.hpp"
#include "Core/Version.hpp"

#include <cstdint>
#include <string>
#include <vector>


namespace PotatoAlert::ReplayParser {

struct ArenaInfoVehicle
{
	uint64_t ShipId;
	uint32_t Relation;
	uint32_t Id;
	std::string Name;
};

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

Core::JsonResult<ReplayMeta> ParseMeta(std::string_view str);

}  // namespace PotatoAlert::ReplayParser
