// Copyright 2025 <github.com/razaqq>
#pragma once

#include "Core/Json.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <optional>
#include <vector>

namespace PotatoAlert::Client::StatsParser {

using ColorRGB = std::array<uint8_t, 3>;
using ColorRGBA = std::array<uint8_t, 4>;

struct Stat
{
	std::string Str;
	ColorRGB ColorRGB;
};

struct Clan
{
	std::string Name;
	std::string Tag;
	ColorRGB ColorRGB;
	std::string Region;
};

struct Ship
{
	std::string Name;
	std::string Class;
	std::string Nation;
	uint8_t Tier;
};

struct Player
{
	std::optional<Clan> Clan;
	bool HiddenPro;
	std::string Name;
	ColorRGB NameColor;
	std::optional<Ship> Ship;
	Stat Battles;
	Stat Winrate;
	Stat AvgDmg;
	Stat BattlesShip;
	Stat WinrateShip;
	Stat AvgDmgShip;
	std::optional<Stat> Karma;
	ColorRGBA PrColor;
	std::string WowsNumbers;
	bool IsUsingPa;
};

struct Team
{
	uint8_t Id;
	std::vector<Player> Players;
	Stat AvgDmg;
	Stat AvgWr;
};

struct Match
{
	Team Team1;
	Team Team2;
	std::string MatchGroup;
	std::string StatsMode;
	std::string Region;
	std::string Map;
	std::string DateTime;
};

Core::JsonResult<Match> ParseMatch(std::string_view json);
Core::JsonResult<std::string> ToCSV(const Match& match);

}  // namespace PotatoAlert::Client::StatsParser
