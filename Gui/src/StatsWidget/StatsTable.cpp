// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/ServiceProvider.hpp"
#include "Client/StringTable.hpp"

#include "Gui/LanguageChangeEvent.hpp"
#include "Gui/StatsWidget/StatsTable.hpp"

#include <QApplication>
#include <QEvent>
#include <QHeaderView>
#include <QTableWidgetItem>


using namespace PotatoAlert::Client::StringTable;
using PotatoAlert::Gui::StatsTable;

StatsTable::StatsTable(QWidget* parent) : QTableWidget(parent)
{
	Init();
	InitHeaders();
}

void StatsTable::Init()
{
	qApp->installEventFilter(this);

	setEditTriggers(NoEditTriggers);
	setSelectionMode(NoSelection);
	setFocusPolicy(Qt::NoFocus);
	setAlternatingRowColors(false);

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
		QTableWidgetItem* item = new QTableWidgetItem();
		item->setFont(QFont(QApplication::font().family(), 11));
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

bool StatsTable::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == LanguageChangeEvent::RegisteredType())
	{
		const int lang = dynamic_cast<LanguageChangeEvent*>(event)->GetLanguage();
		horizontalHeaderItem(0)->setText(GetString(lang, StringTableKey::COLUMN_PLAYER));
		horizontalHeaderItem(1)->setText(GetString(lang, StringTableKey::COLUMN_SHIP));
		horizontalHeaderItem(2)->setText(GetString(lang, StringTableKey::COLUMN_MATCHES));
		horizontalHeaderItem(3)->setText(GetString(lang, StringTableKey::COLUMN_WINRATE));
		horizontalHeaderItem(4)->setText(GetString(lang, StringTableKey::COLUMN_AVERAGE_DAMAGE));
		horizontalHeaderItem(5)->setText(GetString(lang, StringTableKey::COLUMN_MATCHES_SHIP));
		horizontalHeaderItem(6)->setText(GetString(lang, StringTableKey::COLUMN_WINRATE_SHIP));
		horizontalHeaderItem(7)->setText(GetString(lang, StringTableKey::COLUMN_AVERAGE_DAMAGE_SHIP));
	}
	return QTableWidget::eventFilter(watched, event);
}
