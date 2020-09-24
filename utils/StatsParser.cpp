// Copyright 2020 <github.com/razaqq>

#include "StatsParser.h"
#include <QLabel>
#include <QTableWidgetItem>
#include <QFont>
#include <string>
#include <vector>
#include <variant>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>

using PotatoAlert::StatsParser;
using nlohmann::json;

teamType StatsParser::parseTeam(json& teamJson, std::string& matchGroup)
{
	teamType team;
	auto teamID = teamJson["ID"].get<int>();

	// skip scenario bots, there might be too many for table
	if ((matchGroup == "pve" || matchGroup == "pve_premade") && teamID == 2)
		return team;

	for (auto& playerJson : teamJson["Players"].get<json>())
	{
		playerType player;

		auto playerName = playerJson["Name"].get<std::string>();
		auto hiddenProfile = playerJson["HiddenPro"].get<bool>();
		auto clanJson = playerJson["Clan"].get<json>();
		auto shipJson = playerJson["Ship"].get<json>();

		// get stats & colors
		QString ship, battles, winrate, avgDmg, battlesShip, winrateShip, avgDmgShip;
		std::vector<int> shipC, prC, battlesC, winrateC, avgDmgC, battlesShipC, winrateShipC, avgDmgShipC;

		if (!shipJson.is_null())
		{
			ship = QString::fromStdString(shipJson["Name"].get<std::string>());
			shipC = shipJson["Color"].get<std::vector<int>>();
		}

		if (!hiddenProfile)
		{
			battles = QString::number(playerJson["Battles"].get<int>());
			winrate = QString::fromStdString(floatToString(playerJson["WinRate"].get<float>()));
			avgDmg = QString::number(playerJson["AvgDmg"].get<int>());
			battlesShip = QString::number(playerJson["BattlesShip"].get<int>());
			winrateShip = QString::fromStdString(floatToString(playerJson["WRShip"].get<float>()));
			avgDmgShip = QString::number(playerJson["AvgDmgShip"].get<int>());
			
			
			battlesC = playerJson["BattlesC"].get<std::vector<int>>();
			winrateC = playerJson["WinRateC"].get<std::vector<int>>();
			avgDmgC = playerJson["AvgDmgC"].get<std::vector<int>>();
			battlesShipC = playerJson["BattlesShipC"].get<std::vector<int>>();
			winrateShipC = playerJson["WRShipC"].get<std::vector<int>>();
			avgDmgShipC = playerJson["AvgDmgShipC"].get<std::vector<int>>();

			prC = playerJson["PrC"].get<std::vector<int>>();
		}
		std::vector<QString> values = { ship, battles, winrate, avgDmg, battlesShip, winrateShip, avgDmgShip };
		std::vector<std::vector<int>> colors = { shipC, battlesC, winrateC, avgDmgC, battlesShipC, winrateShipC, avgDmgShipC };
		assert(values.size() == colors.size());

		// get player name widget
		if (matchGroup != "clan" && !clanJson.is_null() && !clanJson["Name"].get<std::string>().empty())
			player.push_back(nameClanTag(playerName, prC, clanJson));
		else
			player.push_back(nameNoClanTag(playerName, prC));

		QFont font13("Segoe UI", 1, QFont::Bold);
		font13.setPixelSize(13);
		QFont font16("Segoe UI", 1, QFont::Bold);
		font16.setPixelSize(16);

		QColor background;
		if (!hiddenProfile && StatsParser::validColor(prC))
			background.setRgb(prC[0], prC[1], prC[2], prC[3]);

		for (int i = 0; i < values.size(); i++)
		{
			auto item = new QTableWidgetItem(values[i]);
			
			if (i == 0) {
				item->setFont(font13);
				item->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
			}
			else {
				item->setFont(font16);
				item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
			}

			if (background.isValid())
				item->setBackground(background);

			if (StatsParser::validColor(colors[i]))
				item->setForeground(QColor::fromRgb(colors[i][0], colors[i][1], colors[i][2]));

			player.push_back(item);
		}

		team.push_back(player);
	}
	return team;
}

fieldType StatsParser::nameClanTag(std::string& playerName, std::vector<int>& prC, json& clanJson)
{
	auto clanTag = clanJson["Tag"].get<std::string>();
	auto clanC = clanJson["Color"].get<std::vector<int>>();

	auto name = new QLabel;
	name->setTextFormat(Qt::RichText);
	name->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	name->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	name->setContentsMargins(3, 0, 3, 0);
	name->setText(QString::fromStdString(fmt::format("<span style=\"color: {};\">[{}]</span>{}", arrToRgbString(clanC), clanTag, playerName)));

	if (!prC.empty())
	{
		name->setAutoFillBackground(true);
		name->setStyleSheet(QString::fromStdString(
			fmt::format("background-color: {}; font-size: 13px; font-family: Segoe UI;", arrToRgbString(prC))
		));
	}
	else
	{
		name->setStyleSheet(QString::fromStdString("font-size: 13px; font-family: Segoe UI;"));
	}
	return name;
}

fieldType StatsParser::nameNoClanTag(std::string& playerName, std::vector<int>& prC)
{
	auto name = new QTableWidgetItem(QString::fromStdString(playerName));
	QFont font("Segoe UI");
	font.setPixelSize(13);
	name->setFont(font);
	name->setTextAlignment(Qt::AlignVCenter);
	if (!prC.empty())
		name->setBackground(QColor::fromRgb(prC[0], prC[1], prC[2], prC[3]));
	return name;
}

std::string StatsParser::arrToRgbString(const std::vector<int>& a)
{
	switch (a.size())
	{
	case 3:
		return fmt::format("rgb({}, {}, {})", a[0], a[1], a[2]);
	case 4:
		return fmt::format("rgba({}, {}, {}, {})", a[0], a[1], a[2], a[3]);
	default:
		return "";
	}
}

bool StatsParser::validColor(const std::vector<int>& color)
{
	if (color.size() < 3)
		return false;
	return std::any_of(color.begin(), color.end(), [](int i){ return i != 0; });
}

std::string StatsParser::floatToString(float f)
{
	return fmt::format("{:.1f}", f);
}

std::vector<QString> StatsParser::parseAvg(json& teamJson)
{
	auto avgWr = QString::fromStdString(floatToString(teamJson["AvgWR"].get<float>())) + "%";
	auto avgWrC = "color: " + QString::fromStdString(arrToRgbString(teamJson["AvgWRC"].get<std::vector<int>>()));
	auto avgDmg = QString::number(teamJson["AvgDmg"].get<int>());
	auto avgDmgC = "color: " + QString::fromStdString(arrToRgbString(teamJson["AvgDmgC"].get <std::vector<int>>()));

	return std::vector<QString>{ avgWr, avgWrC, avgDmg, avgDmgC };
}

std::vector<QString> StatsParser::parseClan(json& teamJson)
{
	json clanJson;
	for (auto& player : teamJson["Players"].get<std::vector<json>>())
	{
		clanJson = player["Clan"].get<json>();
		if (!clanJson.is_null())
		{
			auto tag = "[" + QString::fromStdString(clanJson["Tag"].get<std::string>()) + "] ";
			auto color = "color: " + QString::fromStdString(arrToRgbString(clanJson["Color"].get<std::vector<int>>()));
			auto name = QString::fromStdString(clanJson["Name"].get<std::string>());
			auto region = QString::fromStdString(clanJson["Region"].get<std::string>());

			return std::vector<QString>{ tag, color, name, region };
		}
	}
	return std::vector<QString>{};
}

std::vector<QString> StatsParser::parseWowsNumbersProfile(json& teamJson)
{
	std::vector<QString> team;
	for (auto& player : teamJson["Players"].get<std::vector<json>>())
	{
		auto wowsNumbersLink = player["WowsNumbers"].get<std::string>();
		team.push_back(QString::fromStdString(wowsNumbersLink));
	}
	return team;
}
