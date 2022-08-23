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
	StatsTable* m_leftTable = new StatsTable(this);
	StatsTable* m_rightTable = new StatsTable(this);
	StatsTeamFooter* m_footer = new StatsTeamFooter(this);
	StatsHeader* m_header = new StatsHeader(this);

	MatchType m_lastMatch;
};

}  // namespace PotatoAlert::Gui
