// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>
#include <QTableWidgetItem>
#include <QLabel>
#include <QString>
#include <vector>
#include <variant>
#include "StatsTable.hpp"
#include "StatsHeader.hpp"
#include "StatsTeamFooter.hpp"
#include "PotatoClient.hpp"

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
};

}  // namespace PotatoAlert
