// Copyright 2020 <github.com/razaqq>

#include "StatsWidget.hpp"
#include "PotatoClient.hpp"
#include "StatsHeader.hpp"
#include "StatsParser.hpp"
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QTableWidgetItem>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>
#include <array>
#include <variant>
#include <vector>


using PotatoAlert::StatsWidget;

StatsWidget::StatsWidget(QWidget* parent) : QWidget(parent)
{
	this->Init();
}

void StatsWidget::Init()
{
	auto vLayout = new QVBoxLayout;
	vLayout->setContentsMargins(0, 0, 0, 10);
	vLayout->setSpacing(0);

	vLayout->addWidget(this->header);

	auto tableLayout = new QHBoxLayout;
	tableLayout->addWidget(this->left);
	tableLayout->addWidget(this->right);
	tableLayout->setSpacing(10);
	tableLayout->setContentsMargins(10, 0, 10, 0);
	vLayout->addLayout(tableLayout);

	vLayout->addWidget(this->footer);

	this->setLayout(vLayout);
}

void StatsWidget::Update(const Match& match)
{
	this->lastMatch = match;

	// update the tables
	this->left->clearContents();
	this->right->clearContents();

	auto fillTable = [](QTableWidget* table, const TeamType& team)
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

	fillTable(this->left, match.team1.table);
	fillTable(this->right, match.team2.table);

	// update the footer
	this->footer->Update(match);

	// add hooks to open wows-numbers link when double clicking cell
	auto openWowsNumbers = [](int row, const Team& team)
	{
		auto wowsNumbers = team.wowsNumbers;

		if (row < team.wowsNumbers.size())
		{
			QUrl url(team.wowsNumbers[row]);
			if (url.isValid())
				QDesktopServices::openUrl(url);
		}
	};
	connect(this->left, &StatsTable::cellDoubleClicked, [&](int row, int column)
	{
		openWowsNumbers(row, this->lastMatch.team1);
	});
	connect(this->right, &StatsTable::cellDoubleClicked, [&](int row, int column)
	{
		openWowsNumbers(row, this->lastMatch.team2);
	});
}

void StatsWidget::SetStatus(Status status, const std::string& statusText)
{
	this->header->SetStatus(status, statusText);
}
