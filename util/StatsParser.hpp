// Copyright 2021 <github.com/razaqq>

#pragma once

#include "Json.hpp"

#include <QColor>
#include <QLabel>
#include <QTableWidgetItem>

#include <optional>
#include <variant>
#include <vector>


namespace PotatoAlert::StatsParser
{

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
};

bool ParseMatch(const std::string& raw, Match& outMatch, bool saveCSV) noexcept;

}  // namespace PotatoAlert::StatsParser
