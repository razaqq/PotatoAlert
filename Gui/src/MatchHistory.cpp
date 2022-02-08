// Copyright 2020 <github.com/razaqq>

#include "Client/MatchHistory.hpp"

#include "Core/StringTable.hpp"

#include "Gui/MatchHistory.hpp"

#include <QDateTime>
#include <QHeaderView>
#include <QPainter>
#include <QTableWidget>
#include <QVBoxLayout>


using namespace PotatoAlert::Core;
using PotatoAlert::Gui::MatchHistory;

void MatchHistory::paintEvent(QPaintEvent* _)
{
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this); 
}

MatchHistory::MatchHistory(QWidget* parent) : QWidget(parent)
{
	Init();
	InitHeaders();
	UpdateAll();
}

void MatchHistory::Init()
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	
	m_table->setObjectName("matchHistoryWidget");

	auto centralLayout = new QHBoxLayout();
	centralLayout->setContentsMargins(10, 10, 10, 10);
	centralLayout->setSpacing(0);

	centralLayout->addStretch();
	centralLayout->addWidget(m_table);
	centralLayout->addStretch();

	m_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	
	m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_table->setSelectionMode(QAbstractItemView::SingleSelection);
	m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_table->setFocusPolicy(Qt::NoFocus);
	m_table->setAlternatingRowColors(false);

	m_table->setColumnCount(8);
	m_table->hideColumn(7);
	m_table->setSortingEnabled(true);
	m_table->setContentsMargins(0, 0, 0, 0);
	m_table->setCursor(Qt::PointingHandCursor);

	connect(m_table, &QTableWidget::cellDoubleClicked, [this](int row, int _)
	{
		if (const auto item = m_table->item(row, 7))
		{
			auto match = Client::MatchHistory::Instance().GetMatchJson(item->data(Qt::DisplayRole).toInt());
			if (match.has_value())
			{
				emit ReplaySelected(match.value());
			}
		}
	});

	setLayout(centralLayout);
}

void MatchHistory::InitHeaders() const
{
	for (int i = 0; i < m_table->columnCount(); i++)
	{
		auto item = new QTableWidgetItem();
		item->setFont(QFont("Segoe UI", 11));
		m_table->setHorizontalHeaderItem(i, item);
	}

	m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	m_table->setMinimumWidth(800);

	m_table->horizontalHeader()->setVisible(true);
	m_table->verticalHeader()->setVisible(false);
	m_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_table->verticalHeader()->setDefaultSectionSize(20);
	m_table->setCursor(Qt::PointingHandCursor);
}

void MatchHistory::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		m_table->horizontalHeaderItem(0)->setText(StringTable::GetString(StringTable::Keys::HISTORY_DATE));
		m_table->horizontalHeaderItem(1)->setText(StringTable::GetString(StringTable::Keys::COLUMN_SHIP));
		m_table->horizontalHeaderItem(2)->setText(StringTable::GetString(StringTable::Keys::HISTORY_MAP));
		m_table->horizontalHeaderItem(3)->setText(StringTable::GetString(StringTable::Keys::HISTORY_MODE));
		m_table->horizontalHeaderItem(4)->setText(StringTable::GetString(StringTable::Keys::SETTINGS_STATS_MODE));
		m_table->horizontalHeaderItem(5)->setText(StringTable::GetString(StringTable::Keys::COLUMN_PLAYER));
		m_table->horizontalHeaderItem(6)->setText(StringTable::GetString(StringTable::Keys::HISTORY_REGION));
	}
	else
	{
		QWidget::changeEvent(event);
	}
}

void MatchHistory::UpdateAll()
{
	m_table->clearContents();
	m_table->setRowCount(0);

	m_table->setSortingEnabled(false);
	for (auto& entry : Client::MatchHistory::Instance().GetEntries())
	{
		AddEntry(entry);
	}
	m_table->sortByColumn(0, Qt::SortOrder::DescendingOrder);
	m_table->setSortingEnabled(true);
}

void MatchHistory::UpdateLatest()
{
	if (auto res = Client::MatchHistory::Instance().GetLatestEntry())
	{
		const Client::MatchHistory::Entry entry = res.value();

		// check that we don't have that entry already
		if (m_entries.empty() || entry.Id > m_entries.rbegin()->first)
		{
			m_table->setSortingEnabled(false);
			AddEntry(entry);
			m_table->sortByColumn(0, Qt::SortOrder::DescendingOrder);
			m_table->setSortingEnabled(true);
		}
	}
}

void MatchHistory::AddEntry(const Client::MatchHistory::Entry& entry)
{
	m_table->insertRow(m_table->rowCount());
	const int row = m_table->rowCount() - 1;

	QTableWidgetItem* dateItem = new QTableWidgetItem();
	const auto date = QDateTime::fromString(QString::fromStdString(entry.Date), "dd.MM.yyyy hh:mm:ss");
	dateItem->setData(Qt::DisplayRole, date);

	std::array fields = {
		dateItem,
		new QTableWidgetItem(QString::fromStdString(entry.Ship)),
		new QTableWidgetItem(QString::fromStdString(entry.Map)),
		new QTableWidgetItem(QString::fromStdString(entry.MatchGroup)),
		new QTableWidgetItem(QString::fromStdString(entry.StatsMode)),
		new QTableWidgetItem(QString::fromStdString(entry.Player)),
		new QTableWidgetItem(QString::fromStdString(entry.Region))
	};

	for (size_t i = 0; i < fields.size(); i++)
	{
		m_table->setItem(row, i, fields[i]);
	}

	m_entries[entry.Id] = std::move(fields);

	auto item = new QTableWidgetItem();
	item->setData(Qt::DisplayRole, entry.Id);
	m_table->setItem(row, 7, item);

	SetSummary(entry.Id, entry.ReplaySummary);
}

void MatchHistory::SetSummary(uint32_t id, const ReplaySummary& summary) const
{
	if (m_entries.contains(id))
	{
		QColor bg;
		switch (summary.Outcome)
		{
			case ReplayParser::MatchOutcome::Win:
				bg = QColor::fromRgb(23, 209, 51, 50);
				break;
			case ReplayParser::MatchOutcome::Loss:
				bg = QColor::fromRgb(254, 14, 0, 50);
				break;
			case ReplayParser::MatchOutcome::Draw:
				bg = QColor::fromRgb(255, 199, 31, 50);
				break;
			case ReplayParser::MatchOutcome::Unknown:
				return;
		}

		for (const auto& item : m_entries.at(id))
		{
			item->setBackground(bg);
		}
	}
}
