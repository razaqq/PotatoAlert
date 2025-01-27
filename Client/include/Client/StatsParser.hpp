// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Json.hpp"

#include <QColor>
#include <QLabel>
#include <QTableWidgetItem>

#include <optional>
#include <variant>
#include <vector>


namespace PotatoAlert::Client {

namespace StatsParser {

typedef std::variant<QLabel*, QTableWidgetItem*, QWidget*> FieldType;
typedef std::vector<FieldType> PlayerType;
typedef std::vector<PlayerType> TeamType;
typedef std::vector<QString> WowsNumbersType;

struct Label
{
	QString Text;
	std::optional<QColor> Color;

	void UpdateLabel(QLabel* label) const;
};

struct Team
{
	TeamType Table;
	WowsNumbersType WowsNumbers;

	// averages
	Label AvgDmg;
	Label Winrate;

	// clan wars
	struct
	{
		bool Show = false;
		Label Tag;
		Label Name;
		Label Region;
	} Clan;
};

struct MatchType
{
	Team Team1;
	Team Team2;

	struct InfoType
	{
		std::string Map;
		std::string ShipIdent;
		std::string ShipName;
		std::string ShipClass;
		std::string ShipNation;
		uint8_t ShipTier;
		std::string DateTime;
		std::string MatchGroup;
		std::string StatsMode;
		std::string Region;
		std::string Player;
	} Info;
};

struct StatsParseResult
{
	MatchType Match;
	std::string Csv;
};

struct MatchContext
{
	std::string ArenaInfo;
	std::string PlayerName;
	std::string ShipIdent;
};

struct MatchParseOptions
{
	bool ShowKarma;
	bool FontShadow;
	float FontScaling;
};

Core::JsonResult<StatsParseResult> ParseMatch(const rapidjson::Value& j, const MatchContext& matchContext, MatchParseOptions&& parseOptions) noexcept;
Core::JsonResult<StatsParseResult> ParseMatch(const std::string& raw, const MatchContext& matchContext, MatchParseOptions&& parseOptions) noexcept;

}  // namespace StatsParser

}  // namespace PotatoAlert::Client
