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
	Q_OBJECT

public:
	enum class Side
	{
		Friendly,
		Enemy,
	};

	explicit TeamWidget(Side side, QWidget* parent = nullptr);

	void Update(const Team& team);
	void SetStatus(Client::Status status, std::string_view text) const;
	QRect GetPlayerColumnRect(QWidget* parent) const;
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	Side m_side;
	StatsTable* m_table;
	StatsTeamFooter* m_footer = new StatsTeamFooter();
	QWidget* m_header;
	Client::StatsParser::WowsNumbersType m_wowsNumbers;

signals:
	void ForceRefresh();
};

}  // namespace PotatoAlert::Gui
