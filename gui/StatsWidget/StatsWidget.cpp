// Copyright 2020 <github.com/razaqq>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QTableWidgetItem>
#include <vector>
#include <variant>
#include "StatsWidget.h"
#include "StatsHeader.h"


using PotatoAlert::StatsWidget;

StatsWidget::StatsWidget(QWidget* parent) : QWidget(parent)
{
	this->init();
}

void StatsWidget::init()
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

	this->setAverages();
	this->setClans();
}

void StatsWidget::fillTables(std::vector<teamType> teams)
{
	this->left->clearContents();
	this->right->clearContents();

	for (int t = 0; t < teams.size(); t++)
	{
		QTableWidget* table;
		if (t == 0)
			table = this->left;
		else
			table = this->right;

		auto team = teams[t];
		for (int r = 0; r < team.size(); r++)
		{
			auto player = team[r];
			for (int c = 0; c < player.size(); c++)
			{
				auto field = player[c];

				if (std::holds_alternative<QTableWidgetItem*>(field))
					table->setItem(r, c, std::get<QTableWidgetItem*>(field));
				if (std::holds_alternative<QLabel*>(field))
					table->setCellWidget(r, c, std::get<QLabel*>(field));		
			}
		}
	}
}

void StatsWidget::setStatus(int statusID, const std::string& statusText)
{
	this->header->setStatus(statusID, statusText);
}

void StatsWidget::setAverages(const std::vector<QString>& avg)
{
	this->footer->setAverages(avg);
}

void StatsWidget::setClans(const std::vector<QString>& clans)
{
	this->footer->setClans(clans);
}

void StatsWidget::setWowsNumbers(const std::vector<std::vector<QString>>& wowsNumbers)
{
	this->left->setWowsNumbers(wowsNumbers[0]);
	this->right->setWowsNumbers(wowsNumbers[1]);
}
