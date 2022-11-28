// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/DatabaseManager.hpp"
#include "Client/StatsParser.hpp"
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
using PotatoAlert::Client::Config;
using PotatoAlert::Client::DatabaseManager;
using PotatoAlert::Client::Match;
using PotatoAlert::Client::SqlResult;
using PotatoAlert::Client::StatsParser::MatchContext;
using PotatoAlert::Client::StatsParser::StatsParseResult;
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
	m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
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
	m_table->setFocusPolicy(Qt::StrongFocus);

	connect(m_table, &QTableWidget::cellDoubleClicked, [this](int row, int _)
	{
		if (QTableWidgetItem* item = m_table->item(row, m_jsonColumn))
		{
			const int id = item->data(Qt::DisplayRole).toInt();

			PA_TRY_OR_ELSE(const std::optional<std::string> match, m_services.Get<DatabaseManager>().GetMatchJson(id),
			{
				LOG_ERROR("Failed to get match json from database: {}", error);
				return;
			});

			if (match.has_value())
			{
				if (const StatsParseResult res = ParseMatch(match.value(), MatchContext{}); res.Success)
				{
					emit ReplaySelected(res.Match);
				}
			}
		}
	});

	connect(m_deleteButton, &QPushButton::clicked, [this](bool _)
	{
		QItemSelectionModel* select = m_table->selectionModel();

		if (select->hasSelection())
		{
			int lang = m_services.Get<Config>().Get<Client::ConfigKey::Language>();
			auto dialog = new QuestionDialog(lang, this, GetString(lang, StringTableKey::HISTORY_DELETE_QUESTION));
			if (dialog->Run() == QuestionAnswer::Yes)
			{
				std::vector<uint32_t> ids{};
				ids.reserve(select->selectedRows().count());

				// have to sort and remove the rows from high to low
				QList<QModelIndex> indices = select->selectedRows();
				std::ranges::sort(indices, [](const QModelIndex& a, const QModelIndex& b)
				{
					return a.row() > b.row();
				});

				for (const QModelIndex& index : indices)
				{
					uint32_t id = m_table->item(index.row(), m_jsonColumn)->data(Qt::DisplayRole).toInt();
					m_table->removeRow(index.row());
					ids.emplace_back(id);
					m_entries.erase(id);
				}

				PA_TRYV_OR_ELSE(m_services.Get<DatabaseManager>().DeleteMatches(ids),
				{
					LOG_ERROR("Failed to delete matches from match history: {}", error);
				});
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

	PA_TRY_OR_ELSE(const std::vector<Match> matches, m_services.Get<DatabaseManager>().GetMatches(),
	{
		LOG_ERROR("Failed to get matches from database: {}", error);
		return;
	});

	for (const Match& match : matches)
	{
		AddMatch(match);
	}
	m_table->sortByColumn(0, Qt::SortOrder::DescendingOrder);
	m_table->setSortingEnabled(true);
}

void MatchHistory::UpdateLatest()
{
	PA_TRY_OR_ELSE(const std::optional<Match> match, m_services.Get<DatabaseManager>().GetLatestMatch(),
	{
		LOG_ERROR("Failed to get latest match from database: {}", error);
		return;
	});

	if (match.has_value())
	{
		// check that we don't have that entry already
		if (m_entries.empty() || match.value().Id > m_entries.rbegin()->first)
		{
			m_table->setSortingEnabled(false);
			AddMatch(match.value());
			m_table->sortByColumn(0, Qt::SortOrder::DescendingOrder);
			m_table->setSortingEnabled(true);
		}
	}
}

void MatchHistory::AddMatch(const Match& match)
{
	m_table->insertRow(m_table->rowCount());
	const int row = m_table->rowCount() - 1;

	QTableWidgetItem* dateItem = new QTableWidgetItem();
	const auto date = QDateTime::fromString(QString::fromStdString(match.Date), "dd.MM.yyyy hh:mm:ss");
	dateItem->setData(Qt::DisplayRole, date);

	const std::array fields = {
		dateItem,
		new QTableWidgetItem(QString::fromStdString(match.Ship)),
		new QTableWidgetItem(QString::fromStdString(match.Map)),
		new QTableWidgetItem(QString::fromStdString(match.MatchGroup)),
		new QTableWidgetItem(QString::fromStdString(match.StatsMode)),
		new QTableWidgetItem(QString::fromStdString(match.Player)),
		new QTableWidgetItem(QString::fromStdString(match.Region))
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

	const uint32_t matchId = match.Id;
	connect(btn, &QPushButton::clicked, [matchId, this](bool _)
	{
		PA_TRY_OR_ELSE(const std::optional<Match> match, m_services.Get<DatabaseManager>().GetMatch(matchId),
		{
			LOG_ERROR("Failed to get match with id '{}' from database: {}", matchId, error);
			return;
		});

		if (match.has_value())
		{
			emit ReplaySummarySelected(match.value());
		}
	});
	
	m_entries[match.Id] = GuiEntry{ fields, btn };

	auto item = new QTableWidgetItem();
	item->setData(Qt::DisplayRole, match.Id);
	m_table->setItem(row, m_jsonColumn, item);

	if (match.Analyzed)
	{
		SetSummary(match.Id, match.ReplaySummary);
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
