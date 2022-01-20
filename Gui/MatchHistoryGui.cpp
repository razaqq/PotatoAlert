// Copyright 2020 <github.com/razaqq>

#include "MatchHistoryGui.hpp"

#include "MatchHistory.hpp"
#include "StringTable.hpp"

#include <QDateTime>
#include <QHeaderView>
#include <QPainter>
#include <QTableWidget>
#include <QVBoxLayout>


using PotatoAlert::Client::MatchHistory;
using PotatoAlert::Gui::MatchHistoryGui;

void MatchHistoryGui::paintEvent(QPaintEvent* _)
{
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

MatchHistoryGui::MatchHistoryGui(QWidget* parent) : QWidget(parent)
{
	this->Init();
	this->InitHeaders();
	this->UpdateAll();
}

void MatchHistoryGui::Init()
{
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	
	this->m_table->setObjectName("matchHistoryWidget");

	auto centralLayout = new QHBoxLayout();
	centralLayout->setContentsMargins(10, 10, 10, 10);
	centralLayout->setSpacing(0);

	centralLayout->addStretch();
	centralLayout->addWidget(this->m_table);
	centralLayout->addStretch();

	this->m_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	
	this->m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	this->m_table->setSelectionMode(QAbstractItemView::SingleSelection);
	this->m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->m_table->setFocusPolicy(Qt::NoFocus);
	this->m_table->setAlternatingRowColors(false);

	this->m_table->setColumnCount(8);
	this->m_table->hideColumn(7);
	this->m_table->setSortingEnabled(true);
	this->m_table->setContentsMargins(0, 0, 0, 0);
	this->m_table->setCursor(Qt::PointingHandCursor);

	connect(this->m_table, &QTableWidget::cellDoubleClicked, [this](int row, int _)
	{
		if (const auto item = this->m_table->item(row, 7))
		{
			auto match = MatchHistory::Instance().GetMatch(item->data(Qt::DisplayRole).toInt());
			if (match.has_value())
			{
				emit this->ReplaySelected(match.value());
			}	
		}
	});

	this->setLayout(centralLayout);
}

void MatchHistoryGui::InitHeaders() const
{
	for (int i = 0; i < this->m_table->columnCount(); i++)
	{
		auto item = new QTableWidgetItem();
		item->setFont(QFont("Segoe UI", 11));
		this->m_table->setHorizontalHeaderItem(i, item);
	}

	this->m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	this->m_table->setMinimumWidth(800);

	this->m_table->horizontalHeader()->setVisible(true);
	this->m_table->verticalHeader()->setVisible(false);
	this->m_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	this->m_table->verticalHeader()->setDefaultSectionSize(20);
	this->m_table->setCursor(Qt::PointingHandCursor);
}

void MatchHistoryGui::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		this->m_table->horizontalHeaderItem(0)->setText(StringTable::GetString(StringTable::Keys::HISTORY_DATE));
		this->m_table->horizontalHeaderItem(1)->setText(StringTable::GetString(StringTable::Keys::COLUMN_SHIP));
		this->m_table->horizontalHeaderItem(2)->setText(StringTable::GetString(StringTable::Keys::HISTORY_MAP));
		this->m_table->horizontalHeaderItem(3)->setText(StringTable::GetString(StringTable::Keys::HISTORY_MODE));
		this->m_table->horizontalHeaderItem(4)->setText(StringTable::GetString(StringTable::Keys::SETTINGS_STATS_MODE));
		this->m_table->horizontalHeaderItem(5)->setText(StringTable::GetString(StringTable::Keys::COLUMN_PLAYER));
		this->m_table->horizontalHeaderItem(6)->setText(StringTable::GetString(StringTable::Keys::HISTORY_REGION));
	}
	else
	{
		QWidget::changeEvent(event);
	}
}

void MatchHistoryGui::UpdateAll()
{
	this->m_table->clearContents();
	this->m_table->setRowCount(0);

	auto matches = MatchHistory::Instance().GetEntries();

	this->m_table->setSortingEnabled(false);
	for (auto& match : matches)
	{
		if (match.id > this->m_latestId)
			this->m_latestId = match.id;
		this->AddEntry(match);
	}
	this->m_table->sortByColumn(0, Qt::SortOrder::DescendingOrder);
	this->m_table->setSortingEnabled(true);
}

void MatchHistoryGui::UpdateLatest()
{
	auto res = MatchHistory::Instance().GetLatest();
	if (res)
	{
		const MatchHistory::MatchHistoryEntry entry = res.value();

		// check that we don't have that entry already somehow
		if (entry.id > this->m_latestId)
		{
			this->m_latestId = entry.id;
			this->m_table->setSortingEnabled(false);
			this->AddEntry(entry);
			this->m_table->sortByColumn(0, Qt::SortOrder::DescendingOrder);
			this->m_table->setSortingEnabled(true);
		}
	}
}

void MatchHistoryGui::AddEntry(const MatchHistory::MatchHistoryEntry& entry) const
{
	this->m_table->insertRow(this->m_table->rowCount());
	const int row = this->m_table->rowCount() - 1;

	auto dateItem = new QTableWidgetItem();
	const auto date = QDateTime::fromString(QString::fromStdString(entry.date), "dd.MM.yyyy hh:mm:ss");
	dateItem->setData(Qt::DisplayRole, date);
	
	this->m_table->setItem(row, 0, dateItem);
	this->m_table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(entry.ship)));
	this->m_table->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(entry.map)));
	this->m_table->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(entry.matchGroup)));
	this->m_table->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(entry.statsMode)));
	this->m_table->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(entry.player)));
	this->m_table->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(entry.region)));
	auto item = new QTableWidgetItem();
	item->setData(Qt::DisplayRole, entry.id);
	this->m_table->setItem(row, 7, item);
}
