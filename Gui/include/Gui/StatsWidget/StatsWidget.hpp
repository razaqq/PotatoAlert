// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/PotatoClient.hpp"
#include "Client/StatsParser.hpp"
#include "StatsHeader.hpp"
#include "StatsTable.hpp"
#include "StatsTeamFooter.hpp"

#include <QWidget>


namespace PotatoAlert::Gui {

class StatsWidget : public QWidget
{
public:
	explicit StatsWidget(QWidget* parent = nullptr);

	void Update(const MatchType& match);
	void SetStatus(Client::Status status, const std::string& statusText) const;
private:
	void Init();
	StatsTable* m_leftTable = new StatsTable();
	StatsTable* m_rightTable = new StatsTable();
	StatsTeamFooter* m_footer = new StatsTeamFooter();
	StatsHeader* m_header = new StatsHeader();

	MatchType m_lastMatch;
};

}  // namespace PotatoAlert::Gui
