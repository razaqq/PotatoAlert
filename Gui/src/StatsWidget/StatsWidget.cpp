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
	this->Init();
}

void StatsWidget::Init()
{
	this->setMinimumWidth(1000);
	auto vLayout = new QVBoxLayout();
	vLayout->setContentsMargins(0, 0, 0, 10);
	vLayout->setSpacing(0);

	vLayout->addWidget(this->m_header);

	auto tableLayout = new QHBoxLayout();
	tableLayout->addWidget(this->m_leftTable);
	tableLayout->addWidget(this->m_rightTable);
	tableLayout->setSpacing(10);
	tableLayout->setContentsMargins(10, 0, 10, 0);
	vLayout->addLayout(tableLayout);

	vLayout->addWidget(this->m_footer);

	this->setLayout(vLayout);


	// add hooks to open wows-numbers link when double clicking cell
	auto openWowsNumbers = [](int row, const Client::StatsParser::Team& team)
	{
		auto wowsNumbers = team.wowsNumbers;

		if (static_cast<size_t>(row) < team.wowsNumbers.size())
		{
			const QUrl url(team.wowsNumbers[row]);
			if (url.isValid())
				QDesktopServices::openUrl(url);
		}
	};
	connect(this->m_leftTable, &StatsTable::cellDoubleClicked, [&](int row, [[maybe_unused]] int column)
	{
		openWowsNumbers(row, this->m_lastMatch.team1);
	});
	connect(this->m_rightTable, &StatsTable::cellDoubleClicked, [&](int row, [[maybe_unused]] int column)
	{
		openWowsNumbers(row, this->m_lastMatch.team2);
	});
}

void StatsWidget::Update(const Match& match)
{
	this->m_lastMatch = match;

	// update the tables
	this->m_leftTable->clearContents();
	this->m_rightTable->clearContents();

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
				col++;
			}
			row++;
		}
	};

	fillTable(this->m_leftTable, match.team1.table);
	fillTable(this->m_rightTable, match.team2.table);

	// update the footer
	this->m_footer->Update(match);
}

void StatsWidget::SetStatus(Status status, const std::string& statusText) const
{
	this->m_header->SetStatus(status, statusText);
}
