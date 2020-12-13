// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QString>
#include <QLabel>
#include <QTableWidgetItem>
#include <string>
#include <vector>
#include <variant>
#include <nlohmann/json.hpp>


using nlohmann::json;

typedef std::variant<QLabel*, QTableWidgetItem*> fieldType;
typedef std::vector<fieldType> playerType;
typedef std::vector<playerType> teamType;

namespace PotatoAlert {

class StatsParser
{
public:
	static teamType parseTeam(json& teamJson, std::string& matchGroup);
	static std::vector<QString> parseAvg(json& teamJson);
	static std::vector<QString> parseClan(json& clanJson);
	static std::vector<QString> parseWowsNumbersProfile(json& teamJson);
private:
	static fieldType nameClanTag(std::string& playerName, std::vector<int>& prC, json& clanJson);
	static fieldType nameNoClanTag(std::string& playerName, std::vector<int>& prC);
	
	// helpers
	static std::string arrToRgbString(const std::vector<int>& a);
	static bool validColor(const std::vector<int>& color);
	static std::string floatToString(float f);
};

}  // namespace PotatoAlert
