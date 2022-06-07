// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Json.hpp"

#include <QColor>
#include <QLabel>
#include <QTableWidgetItem>

#include <optional>
#include <variant>
#include <vector>


namespace PotatoAlert::Client::StatsParser {

typedef std::variant<QLabel*, QTableWidgetItem*> FieldType;
typedef std::vector<FieldType> PlayerType;
typedef std::vector<PlayerType> TeamType;
typedef std::vector<QString> WowsNumbersType;

struct Label
{
	QString text;
	std::optional<QColor> color;

	void UpdateLabel(QLabel* label) const;
};

struct Team
{
	TeamType table;
	WowsNumbersType wowsNumbers;

	// averages
	Label avgDmg;
	Label winrate;

	// clan wars
	struct
	{
		bool show = false;
		Label tag;
		Label name;
		Label region;
	} clan;
};

struct Match
{
	Team team1;
	Team team2;

	struct Info
	{
		std::string map;
		std::string ship;
		std::string shipIdent;
		std::string dateTime;
		std::string matchGroup;
		std::string statsMode;
		std::string region;
		std::string player;
	} info;
};

struct StatsParseResult
{
	bool success = false;
	Match match;
	std::optional<std::string> csv;
};

struct MatchContext
{
	std::string ArenaInfo;
	std::string PlayerName;
	std::string ShipIdent;
};

StatsParseResult ParseMatch(const json& j, const MatchContext& matchContext, bool parseCsv) noexcept;
StatsParseResult ParseMatch(const std::string& raw, const MatchContext& matchContext, bool parseCsv) noexcept;

}  // namespace PotatoAlert::Client::StatsParser
