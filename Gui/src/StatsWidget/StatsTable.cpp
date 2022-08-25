// Copyright 2020 <github.com/razaqq>

#include "Client/StringTable.hpp"

#include "Gui/StatsWidget/StatsTable.hpp"

#include <QEvent>
#include <QHeaderView>
#include <QTableWidgetItem>


using namespace PotatoAlert::Core;
using PotatoAlert::Gui::StatsTable;

StatsTable::StatsTable(QWidget* parent) : QTableWidget(parent)
{
	Init();
	InitHeaders();
}

void StatsTable::Init()
{
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setSelectionMode(QAbstractItemView::NoSelection);
	setFocusPolicy(Qt::NoFocus);
	setAlternatingRowColors(false);

	// setMouseTracking(true);

	setRowCount(12);
	setColumnCount(8);
	setSortingEnabled(false);
	setContentsMargins(0, 0, 0, 0);
	setCursor(Qt::PointingHandCursor);
}

void StatsTable::InitHeaders()
{
	for (int i = 0; i < columnCount(); i++)
	{
		auto item = new QTableWidgetItem();
		item->setFont(QFont("Segoe UI", 11));
		setHorizontalHeaderItem(i, item);
	}
	
	QHeaderView* hHeaders = horizontalHeader();
	hHeaders->setSectionResizeMode(QHeaderView::Stretch);
	hHeaders->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	hHeaders->setSectionResizeMode(1, QHeaderView::ResizeToContents);

	horizontalHeader()->setVisible(true);
	verticalHeader()->setVisible(false);
	verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	resizeColumnsToContents();
	setCursor(Qt::PointingHandCursor);
}

void StatsTable::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		horizontalHeaderItem(0)->setText(GetString(StringTable::Keys::COLUMN_PLAYER));
		horizontalHeaderItem(1)->setText(GetString(StringTable::Keys::COLUMN_SHIP));
		horizontalHeaderItem(2)->setText(GetString(StringTable::Keys::COLUMN_MATCHES));
		horizontalHeaderItem(3)->setText(GetString(StringTable::Keys::COLUMN_WINRATE));
		horizontalHeaderItem(4)->setText(GetString(StringTable::Keys::COLUMN_AVERAGE_DAMAGE));
		horizontalHeaderItem(5)->setText(GetString(StringTable::Keys::COLUMN_MATCHES_SHIP));
		horizontalHeaderItem(6)->setText(GetString(StringTable::Keys::COLUMN_WINRATE_SHIP));
		horizontalHeaderItem(7)->setText(GetString(StringTable::Keys::COLUMN_AVERAGE_DAMAGE_SHIP));
	}
	else
	{
		QTableWidget::changeEvent(event);
	}
}
