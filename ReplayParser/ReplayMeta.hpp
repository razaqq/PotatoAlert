// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Json.hpp"
#include "Core/Version.hpp"

#include <string>
#include <unordered_map>
#include <vector>


namespace PotatoAlert::ReplayParser {

struct VehicleInfoMeta
{
	uint64_t shipId;
	uint32_t relation;
	uint32_t id;
	std::string name;
};

[[maybe_unused]] static void from_json(const json& j, VehicleInfoMeta& m)
{
	j.at("shipId").get_to(m.shipId);
	j.at("relation").get_to(m.relation);
	j.at("id").get_to(m.id);
	j.at("name").get_to(m.name);
}

struct ReplayMeta
{
	std::string matchGroup;
	uint32_t gameMode;
	Version clientVersionFromExe;
	uint32_t scenarioUiCategoryId;
	std::string mapDisplayName;
	uint32_t mapId;
	Version clientVersionFromXml;
	std::unordered_map<std::string, std::vector<std::string>> weatherParams;
	//mapBorder null;
	uint32_t duration;
	std::string gameLogic;
	std::string name;
	std::string scenario;
	uint32_t playerID;
	std::vector<VehicleInfoMeta> vehicles;
	uint32_t playersPerTeam;
	std::string dateTime;
	std::string mapName;
	std::string playerName;
	uint32_t scenarioConfigId;
	uint32_t teamsCount;
	std::string logic;
	std::string playerVehicle;
	uint32_t battleDuration;
};

[[maybe_unused]] static void from_json(const json& j, ReplayMeta& m)
{
	j.at("matchGroup").get_to(m.matchGroup);
	j.at("gameMode").get_to(m.gameMode);
	m.clientVersionFromExe = Version(j.at("clientVersionFromExe").get<std::string>());
	j.at("scenarioUiCategoryId").get_to(m.scenarioUiCategoryId);
	j.at("mapDisplayName").get_to(m.mapDisplayName);
	j.at("mapId").get_to(m.mapId);
	m.clientVersionFromXml = Version(j.at("clientVersionFromXml").get<std::string>());
	j.at("weatherParams").get_to(m.weatherParams);
	j.at("duration").get_to(m.duration);
	j.at("gameLogic").get_to(m.gameLogic);
	j.at("name").get_to(m.name);
	j.at("scenario").get_to(m.scenario);
	j.at("playerID").get_to(m.playerID);
	j.at("vehicles").get_to(m.vehicles);
	j.at("playersPerTeam").get_to(m.playersPerTeam);
	j.at("dateTime").get_to(m.dateTime);
	j.at("mapName").get_to(m.mapName);
	j.at("playerName").get_to(m.playerName);
	j.at("scenarioConfigId").get_to(m.scenarioConfigId);
	j.at("teamsCount").get_to(m.teamsCount);
	j.at("logic").get_to(m.logic);
	j.at("playerVehicle").get_to(m.playerVehicle);
	j.at("battleDuration").get_to(m.battleDuration);
}

}  // namespace PotatoAlert::ReplayParser
