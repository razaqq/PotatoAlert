// Copyright 2020 <github.com/razaqq>

#include <QMouseEvent>
#include <QTableWidgetItem>
#include <QFont>
#include <QString>
#include <QAbstractItemView>
#include <QHeaderView>
#include <QModelIndex>
#include <QDesktopServices>
#include <string>
#include <vector>
#include <iostream>
#include "StatsTable.h"


using PotatoAlert::StatsTable;

StatsTable::StatsTable(QWidget* parent) : QTableWidget(parent)
{
	init();
	initHeaders();
}

void StatsTable::init()
{
	this->setEditTriggers(QAbstractItemView::NoEditTriggers);
	this->setSelectionMode(QAbstractItemView::NoSelection);
	this->setFocusPolicy(Qt::NoFocus);
	this->setAlternatingRowColors(false);

	// this->setMouseTracking(true);

	this->setRowCount(12);
	this->setColumnCount(8);
	this->setSortingEnabled(false);
	this->setContentsMargins(0, 0, 0, 0);
	this->setCursor(Qt::PointingHandCursor);
	connect(this, &StatsTable::cellDoubleClicked, this, &StatsTable::clickEvent);
}

void StatsTable::initHeaders()
{
	const std::vector<QString> headerLabels{
		"Player", "Ship", "Matches", "Winrate", "Avg DMG", "M Ship", "WR Ship", "DMG Ship"
	};
	for (int i = 0; i < headerLabels.size(); i++)
	{
		QTableWidgetItem* item = new QTableWidgetItem;
		item->setText(headerLabels[i]);
		item->setFont(QFont("Segoe UI", 11));
		this->setHorizontalHeaderItem(i, item);
	}

	QHeaderView* headers = new QHeaderView(Qt::Horizontal, this);
	this->setHorizontalHeader(headers);
	headers->setSectionResizeMode(QHeaderView::Stretch);
	headers->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	headers->setSectionResizeMode(0, QHeaderView::ResizeToContents);

	this->horizontalHeader()->setVisible(true);
	this->verticalHeader()->setVisible(false);
	this->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	this->resizeColumnsToContents();
	this->setCursor(Qt::PointingHandCursor);
}

void StatsTable::clickEvent(int row, int column)
{
	if (row < this->wowsNumbers.size())
		QDesktopServices::openUrl(QUrl(wowsNumbers[row]));
}

void StatsTable::setWowsNumbers(const std::vector<QString>& wowsNumbers)
{
	this->wowsNumbers = wowsNumbers;
}
