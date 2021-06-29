// Copyright 2020 <github.com/razaqq>
#pragma once

#include "PotatoClient.hpp"
#include "StatsHeader.hpp"
#include "StatsParser.hpp"
#include "StatsTable.hpp"
#include "StatsTeamFooter.hpp"
#include <QLabel>
#include <QString>
#include <QTableWidgetItem>
#include <QWidget>
#include <array>
#include <variant>
#include <vector>


typedef std::vector<std::vector<std::variant<QLabel*, QTableWidgetItem*>>> teamType;

namespace PotatoAlert {

class StatsWidget : public QWidget
{
public:
	explicit StatsWidget(QWidget* parent);

	void Update(const Match& match);
	void SetStatus(Status status, const std::string& statusText);
private:
	void Init();
	StatsTable* left = new StatsTable(this);
	StatsTable* right = new StatsTable(this);
	StatsTeamFooter* footer = new StatsTeamFooter(this);
	StatsHeader* header = new StatsHeader(this);

	Match lastMatch;
};

}  // namespace PotatoAlert
