// Copyright 2020 <github.com/razaqq>

#include <QMouseEvent>
#include <QTableWidgetItem>
#include <QFont>
#include <QEvent>
#include <QString>
#include <QAbstractItemView>
#include <QHeaderView>
#include <QModelIndex>
#include <QDesktopServices>
#include <string>
#include <vector>
#include <iostream>
#include "StatsTable.hpp"
#include "StringTable.hpp"


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
	for (int i = 0; i < this->columnCount(); i++)
	{
		auto item = new QTableWidgetItem;
		item->setFont(QFont("Segoe UI", 11));
		this->setHorizontalHeaderItem(i, item);
	}

	auto headers = new QHeaderView(Qt::Horizontal, this);
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

void StatsTable::clickEvent(int row)
{
	if (row < this->wowsNumbers.size())
		QDesktopServices::openUrl(QUrl(wowsNumbers[row]));
}

void StatsTable::setWowsNumbers(const std::vector<QString>& data)
{
	this->wowsNumbers = data;
}

void StatsTable::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		this->horizontalHeaderItem(0)->setText(GetString(StringKeys::COLUMN_PLAYER));
		this->horizontalHeaderItem(1)->setText(GetString(StringKeys::COLUMN_SHIP));
		this->horizontalHeaderItem(2)->setText(GetString(StringKeys::COLUMN_MATCHES));
		this->horizontalHeaderItem(3)->setText(GetString(StringKeys::COLUMN_WINRATE));
		this->horizontalHeaderItem(4)->setText(GetString(StringKeys::COLUMN_AVERAGE_DAMAGE));
		this->horizontalHeaderItem(5)->setText(GetString(StringKeys::COLUMN_MATCHES_SHIP));
		this->horizontalHeaderItem(6)->setText(GetString(StringKeys::COLUMN_WINRATE_SHIP));
		this->horizontalHeaderItem(7)->setText(GetString(StringKeys::COLUMN_AVERAGE_DAMAGE_SHIP));
	}
	else
	{
		QTableWidget::changeEvent(event);
	}
}
