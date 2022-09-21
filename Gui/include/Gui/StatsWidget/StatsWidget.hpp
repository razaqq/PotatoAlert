// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/PotatoClient.hpp"
#include "Client/StatsParser.hpp"

#include "Gui/StatsWidget/StatsHeader.hpp"
#include "Gui/StatsWidget/StatsTable.hpp"
#include "Gui/StatsWidget/StatsTeamFooter.hpp"

#include <QWidget>


namespace PotatoAlert::Gui {

class StatsWidget : public QWidget
{
private:
	StatsTable* m_leftTable = new StatsTable();
	StatsTable* m_rightTable = new StatsTable();
	StatsTeamFooter* m_footer = new StatsTeamFooter();
	StatsHeader* m_header = new StatsHeader();

	MatchType m_lastMatch;

public:
	explicit StatsWidget(QWidget* parent = nullptr);

	void Update(const MatchType& match);
	void SetStatus(Client::Status status, std::string_view statusText) const;

private:
	void Init();
};

}  // namespace PotatoAlert::Gui
