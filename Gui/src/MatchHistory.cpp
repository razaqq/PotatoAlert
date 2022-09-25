// Copyright 2020 <github.com/razaqq>

#include "Client/MatchHistory.hpp"
#include "Client/PotatoClient.hpp"
#include "Client/StringTable.hpp"

#include "Gui/MatchHistory.hpp"

#include "Gui/LanguageChangeEvent.hpp"
#include "Gui/QuestionDialog.hpp"

#include <QApplication>
#include <QDateTime>
#include <QHeaderView>
#include <QPainter>
#include <QTableWidget>
#include <QVBoxLayout>


using namespace PotatoAlert::Client::StringTable;
using namespace PotatoAlert::Core;
using PotatoAlert::Client::PotatoClient;
using PotatoAlert::Gui::MatchHistory;

void MatchHistory::paintEvent(QPaintEvent* _)
{
	QStyleOption opt;
	opt.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

MatchHistory::MatchHistory(const Client::ServiceProvider& serviceProvider, QWidget* parent) : QWidget(parent), m_services(serviceProvider)
{
	Init();
	InitHeaders();
	UpdateAll();
}

void MatchHistory::Init()
{
	qApp->installEventFilter(this);

	QHBoxLayout* horLayout = new QHBoxLayout();
	horLayout->setContentsMargins(10, 10, 10, 10);
	horLayout->setSpacing(0);
	QWidget* centralWidget = new QWidget(this);
	centralWidget->setObjectName("matchHistoryWidget");
	horLayout->addStretch();
	horLayout->addWidget(centralWidget);
	horLayout->addStretch();
	auto layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 10);
	centralWidget->setLayout(layout);

	/*
	m_analyzeButton->setFixedWidth(100);
	m_analyzeButton->setObjectName("settingsButton");
	m_analyzeButton->setEnabled(false);
	*/

	m_deleteButton->setFixedWidth(100);
	m_deleteButton->setObjectName("settingsButton");
	m_deleteButton->setEnabled(false);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch();
	// buttonLayout->addWidget(m_analyzeButton);
	buttonLayout->addWidget(m_deleteButton);
	buttonLayout->addStretch();

	layout->addWidget(m_table);
	layout->addLayout(buttonLayout);

	m_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_table->setObjectName("matchHistoryTable");
	
	m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_table->setSelectionMode(QAbstractItemView::SingleSelection);
	m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_table->setFocusPolicy(Qt::NoFocus);
	m_table->setAlternatingRowColors(false);

	m_table->setColumnCount(9);
	m_jsonColumn = m_table->columnCount() - 1;
	m_btnColumn = m_jsonColumn - 1;
	m_table->hideColumn(m_jsonColumn);
	m_table->setSortingEnabled(true);
	m_table->setContentsMargins(0, 0, 0, 0);
	m_table->setCursor(Qt::PointingHandCursor);

	connect(m_table, &QTableWidget::cellDoubleClicked, [this](int row, int _)
	{
		if (QTableWidgetItem* item = m_table->item(row, m_jsonColumn))
		{
			const int id = item->data(Qt::DisplayRole).toInt();
			const auto match = m_services.Get<Client::MatchHistory>().GetMatchJson(id);
			if (match.has_value())
			{
				emit ReplaySelected(match.value());
			}
		}
	});

	/*
	connect(m_analyzeButton, &QPushButton::clicked, [this](bool _)
	{
		const int row = m_table->currentRow();
		if (QTableWidgetItem* item = m_table->item(row, 7))
		{
			const int id = item->data(Qt::DisplayRole).toInt();
			Client::MatchHistory::Instance().SetNonAnalyzed(id);
			// TODO: this is a bit hacky, find a way to trigger a run otherwise
			PotatoClient::Instance().CheckPath();
		}
	});
	*/

	connect(m_deleteButton, &QPushButton::clicked, [this](bool _)
	{
		const int row = m_table->currentRow();
		if (QTableWidgetItem* item = m_table->item(row, m_jsonColumn))
		{
			int lang = m_services.Get<Config>().Get<Client::ConfigKey::Language>();
			auto dialog = new QuestionDialog(lang, this, GetString(lang, StringTableKey::HISTORY_DELETE_QUESTION));
			if (dialog->Run() == QuestionAnswer::Yes)
			{
				const int id = item->data(Qt::DisplayRole).toInt();
				m_services.Get<Client::MatchHistory>().DeleteEntry(id);
				m_entries.erase(id);
				m_table->removeRow(row);
			}
		}
	});

	connect(m_table, &QTableWidget::currentCellChanged, [this](int currentRow, int currentColumn, int previousRow, int previousColumn)
	{
		// m_analyzeButton->setEnabled(true);
		m_deleteButton->setEnabled(true);
	});

	setLayout(horLayout);
}

void MatchHistory::InitHeaders() const
{
	for (int i = 0; i < m_table->columnCount(); i++)
	{
		auto item = new QTableWidgetItem();
		item->setFont(QFont("Segoe UI", 11));
		m_table->setHorizontalHeaderItem(i, item);
	}

	m_table->setMinimumWidth(800);
	m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	m_table->horizontalHeader()->setSectionResizeMode(m_btnColumn, QHeaderView::Fixed);
	m_table->horizontalHeader()->setMinimumSectionSize(20);
	// m_table->horizontalHeader()->setMinimumWidth(5);
	m_table->horizontalHeader()->resizeSection(m_btnColumn, 20);
	m_table->horizontalHeader()->setVisible(true);

	m_table->verticalHeader()->setVisible(false);
	m_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_table->verticalHeader()->setDefaultSectionSize(20);
	m_table->setCursor(Qt::PointingHandCursor);
}

bool MatchHistory::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == LanguageChangeEvent::RegisteredType())
	{
		int lang = dynamic_cast<LanguageChangeEvent*>(event)->GetLanguage();
		m_table->horizontalHeaderItem(0)->setText(GetString(lang, StringTableKey::HISTORY_DATE));
		m_table->horizontalHeaderItem(1)->setText(GetString(lang, StringTableKey::COLUMN_SHIP));
		m_table->horizontalHeaderItem(2)->setText(GetString(lang, StringTableKey::HISTORY_MAP));
		m_table->horizontalHeaderItem(3)->setText(GetString(lang, StringTableKey::HISTORY_MODE));
		m_table->horizontalHeaderItem(4)->setText(GetString(lang, StringTableKey::SETTINGS_STATS_MODE));
		m_table->horizontalHeaderItem(5)->setText(GetString(lang, StringTableKey::COLUMN_PLAYER));
		m_table->horizontalHeaderItem(6)->setText(GetString(lang, StringTableKey::HISTORY_REGION));

		// m_analyzeButton->setText(GetString(lang, StringTable::Keys::HISTORY_ANALYZE));
		m_deleteButton->setText(GetString(lang, StringTableKey::HISTORY_DELETE));
	}
	return QWidget::eventFilter(watched, event);
}

void MatchHistory::UpdateAll()
{
	m_table->clearContents();
	m_table->setRowCount(0);

	m_table->setSortingEnabled(false);
	for (auto& entry : m_services.Get<Client::MatchHistory>().GetEntries())
	{
		AddEntry(entry);
	}
	m_table->sortByColumn(0, Qt::SortOrder::DescendingOrder);
	m_table->setSortingEnabled(true);
}

void MatchHistory::UpdateLatest()
{
	if (auto res = m_services.Get<Client::MatchHistory>().GetLatestEntry())
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

	const std::array fields = {
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

	IconButton* btn = new IconButton(":/ReplaySummaryButton.svg", ":/ReplaySummaryButtonHover.svg", QSize(20, 20));
	btn->setCursor(Qt::PointingHandCursor);
	btn->setCheckable(false);
	// btn->setFlat(true);
	m_table->setCellWidget(row, m_btnColumn, btn);

	const uint32_t entryId = entry.Id;
	connect(btn, &QPushButton::clicked, [entryId, this](bool _)
	{
		if (const std::optional<Client::MatchHistory::Entry> entry = m_services.Get<Client::MatchHistory>().GetEntry(entryId))
		{
			emit ReplaySummarySelected(entry.value());
		}
	});
	
	m_entries[entry.Id] = GuiEntry{ fields, btn };

	auto item = new QTableWidgetItem();
	item->setData(Qt::DisplayRole, entry.Id);
	m_table->setItem(row, m_jsonColumn, item);

	if (entry.Analyzed)
	{
		SetSummary(entry.Id, entry.ReplaySummary);
	}
	else
	{
		btn->setEnabled(false);
		btn->HideIcon();
	}
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
				bg = QColor::fromRgb(0, 0, 0, 0);
				break;
		}

		const GuiEntry& entry = m_entries.at(id);
		for (const auto& item : entry.Fields)
		{
			item->setBackground(bg);
		}

		if (!entry.Button->IsIconShown())
		{
			entry.Button->setEnabled(true);
			entry.Button->ShowIcon();
		}

		entry.Button->setStyleSheet(
			std::format("background-color: rgba({}, {}, {}, {});", bg.red(), bg.green(), bg.blue(), bg.alpha()).c_str());
	}
	else
	{
		LOG_ERROR("Tried to set ReplaySummary in MatchHistory for non-existing entry id {}", id);
	}
}
