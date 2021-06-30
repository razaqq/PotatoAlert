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


namespace PotatoAlert {

class StatsWidget : public QWidget
{
public:
	explicit StatsWidget(QWidget* parent = nullptr);

	void Update(const Match& match);
	void SetStatus(Status status, const std::string& statusText);
private:
	void Init();
	StatsTable* m_leftTable = new StatsTable(this);
	StatsTable* m_rightTable = new StatsTable(this);
	StatsTeamFooter* m_footer = new StatsTeamFooter(this);
	StatsHeader* m_header = new StatsHeader(this);

	Match m_lastMatch;
};

}  // namespace PotatoAlert
