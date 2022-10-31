// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Client/PotatoClient.hpp"
#include "Client/StatsParser.hpp"

#include "Gui/StatsWidget/StatsTable.hpp"
#include "Gui/StatsWidget/StatsTeamFooter.hpp"

#include <QWidget>

#include <string>


namespace PotatoAlert::Gui {

class TeamWidget : public QWidget
{
public:
	enum class Side
	{
		Friendly,
		Enemy,
	};

	explicit TeamWidget(Side side, QWidget* parent = nullptr);

	void Update(const Team& team);
	void SetStatus(Client::Status status, std::string_view text) const;

private:
	Side m_side;
	StatsTable* m_table = new StatsTable();
	StatsTeamFooter* m_footer = new StatsTeamFooter();
	QWidget* m_header;
	Client::StatsParser::WowsNumbersType m_wowsNumbers;
};

}  // namespace PotatoAlert::Gui
