// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>
#include <QTableWidgetItem>
#include <QLabel>
#include <QString>
#include <vector>
#include <variant>
#include "StatsTable.h"
#include "StatsHeader.h"
#include "StatsTeamFooter.h"

typedef std::vector<std::vector<std::variant<QLabel*, QTableWidgetItem*>>> teamType;

namespace PotatoAlert {

class StatsWidget : public QWidget
{
public:
	explicit StatsWidget(QWidget* parent);
	void fillTables(std::vector<teamType> teams);
	void setStatus(int statusID, const std::string& statusText);
	void setAverages(const std::vector<QString>& avg = { "0.0%", "", "0", "", "0.0%", "", "0", "" });
	void setClans(const std::vector<QString>& clans = {});
	void setWowsNumbers(const std::vector<std::vector<QString>>& wowsNumbers);
private:
	void init();
	StatsTable* left = new StatsTable(this);
	StatsTable* right = new StatsTable(this);
	StatsTeamFooter* footer = new StatsTeamFooter(this);
	StatsHeader* header = new StatsHeader(this);
};

}  // namespace PotatoAlert
