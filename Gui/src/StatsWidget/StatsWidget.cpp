// Copyright 2020 <github.com/razaqq>

#include "Client/PotatoClient.hpp"
#include "Client/StatsParser.hpp"

#include "Gui/StatsWidget/StatsHeader.hpp"
#include "Gui/StatsWidget/StatsWidget.hpp"

#include <QDesktopServices>
#include <QTableWidgetItem>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include <array>
#include <variant>
#include <vector>


using PotatoAlert::Gui::StatsWidget;

StatsWidget::StatsWidget(QWidget* parent) : QWidget(parent)
{
	Init();
}

void StatsWidget::Init()
{
	setMinimumWidth(1000);
	auto vLayout = new QVBoxLayout();
	vLayout->setContentsMargins(0, 0, 0, 10);
	vLayout->setSpacing(0);

	vLayout->addWidget(m_header);

	auto tableLayout = new QHBoxLayout();
	tableLayout->addWidget(m_leftTable);
	tableLayout->addWidget(m_rightTable);
	tableLayout->setSpacing(10);
	tableLayout->setContentsMargins(10, 0, 10,  0);
	vLayout->addLayout(tableLayout);

	vLayout->addWidget(m_footer);

	setLayout(vLayout);


	// add hooks to open wows-numbers link when double clicking cell
	auto openWowsNumbers = [](int row, const Client::StatsParser::Team& team)
	{
		auto wowsNumbers = team.WowsNumbers;

		if (static_cast<size_t>(row) < team.WowsNumbers.size())
		{
			const QUrl url(team.WowsNumbers[row]);
			if (url.isValid())
				QDesktopServices::openUrl(url);
		}
	};
	connect(m_leftTable, &StatsTable::cellDoubleClicked, [&](int row, [[maybe_unused]] int column)
	{
		openWowsNumbers(row, m_lastMatch.Team1);
	});
	connect(m_rightTable, &StatsTable::cellDoubleClicked, [&](int row, [[maybe_unused]] int column)
	{
		openWowsNumbers(row, m_lastMatch.Team2);
	});
}

void StatsWidget::Update(const MatchType& match)
{
	m_lastMatch = match;

	// update the tables
	m_leftTable->clearContents();
	m_rightTable->clearContents();

	auto fillTable = [](QTableWidget* table, const Client::StatsParser::TeamType& team)
	{
		int row = 0;
		for (auto& player : team)
		{
			int col = 0;
			for (auto& field : player)
			{
				if (std::holds_alternative<QTableWidgetItem*>(field))
					table->setItem(row, col, std::get<QTableWidgetItem*>(field));
				if (std::holds_alternative<QLabel*>(field))
					table->setCellWidget(row, col, std::get<QLabel*>(field));
				if (std::holds_alternative<QWidget*>(field))
					table->setCellWidget(row, col, std::get<QWidget*>(field));
				col++;
			}
			row++;
		}
		table->resizeColumnToContents(1);
	};
	
	fillTable(m_leftTable, match.Team1.Table);
	fillTable(m_rightTable, match.Team2.Table);

	// update the footer
	m_footer->Update(match);
}

void StatsWidget::SetStatus(Status status, std::string_view statusText) const
{
	m_header->SetStatus(status, statusText);
}
