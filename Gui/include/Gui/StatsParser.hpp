// Copyright 2025 <github.com/razaqq>
#pragma once

#include "Client/PotatoClient.hpp"
#include "Client/StatsParser.hpp"

#include "Core/Json.hpp"

#include <QColor>
#include <QLabel>
#include <QString>
#include <QTableWidgetItem>

#include <cstdint>
#include <variant>
#include <vector>


namespace PotatoAlert::Gui::StatsParser {

using FieldType = std::variant<QLabel*, QTableWidgetItem*, QWidget*>;
using PlayerType = std::vector<FieldType>;
using TeamType = std::vector<PlayerType>;
using WowsNumbersType = std::vector<QString>;

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

struct Match
{
	Team Team1;
	Team Team2;
};

struct MatchParseOptions
{
	bool ShowKarma;
	bool FontShadow;
	float FontScaling;
};

Match ParseMatch(const Client::StatsParser::Match& match, const Client::MatchContext& ctx, MatchParseOptions&& parseOptions) noexcept;
Core::JsonResult<Match> ParseMatch(std::string_view json, const Client::MatchContext& ctx, MatchParseOptions&& parseOptions) noexcept;

}  // namespace PotatoAlert::Gui::StatsParser
