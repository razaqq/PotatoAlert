// Copyright 2022 <github.com/razaqq>

#include <Client/Config.hpp>
#include <Client/DatabaseManager.hpp>
#include <Client/ServiceProvider.hpp>
#include <Client/StatsParser.hpp>
#include "Client/StringTable.hpp"

#include "Core/Instrumentor.hpp"
#include "Core/Log.hpp"

#include "Gui/LanguageChangeEvent.hpp"
#include "Gui/MatchHistory/MatchHistory.hpp"
#include "Gui/QuestionDialog.hpp"

#include "Gui/MatchHistory/MatchHistoryModel.hpp"
#include "Gui/MatchHistory/MatchHistorySortFilter.hpp"
#include "Gui/MatchHistory/MatchHistoryView.hpp"
#include "Gui/MatchHistory/ReplaySummaryButtonDelegate.hpp"

#include <QHBoxLayout>
#include <QSortFilterProxyModel>

#include <format>
#include <vector>


using PotatoAlert::Client::Config;
using PotatoAlert::Client::ConfigKey;
using PotatoAlert::Client::DatabaseManager;
using PotatoAlert::Client::StatsParser::MatchContext;
using PotatoAlert::Client::StatsParser::ParseMatch;
using PotatoAlert::Client::StatsParser::StatsParseResult;
using PotatoAlert::Client::StringTable::GetString;
using PotatoAlert::Client::StringTable::StringTableKey;
using PotatoAlert::Gui::MatchHistory;

MatchHistory::MatchHistory(const Client::ServiceProvider& serviceProvider, QWidget* parent) : QWidget(parent), m_services(serviceProvider)
{
	qApp->installEventFilter(this);

	m_view = new MatchHistoryView();
	m_model = new MatchHistoryModel(m_services);

	m_sortFilter = new MatchHistorySortFilter(m_model);
	m_sortFilter->setSourceModel(m_model);

	ReplaySummaryButtonDelegate* summaryButtonDelegate = new ReplaySummaryButtonDelegate();
	connect(summaryButtonDelegate, &ReplaySummaryButtonDelegate::ReplaySummarySelected, [this](const QModelIndex& index)
	{
		emit ReplaySummarySelected(m_model->GetMatch(m_sortFilter->mapToSource(index).row()));
	});

	m_view->setModel(m_sortFilter);
	m_view->setItemDelegateForColumn(MatchHistoryModel::ButtonColumn(), summaryButtonDelegate);
	m_view->sortByColumn(0, Qt::DescendingOrder);

	m_view->Init();

	// maybe we should do this async
	LoadMatches();

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

	m_deleteButton->setFixedWidth(100);
	m_deleteButton->setObjectName("settingsButton");
	m_deleteButton->setEnabled(false);

	m_entryCount->setFixedWidth(150);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->setContentsMargins(15, 0, 15, 0);
	buttonLayout->addWidget(m_entryCount);
	buttonLayout->addStretch();
	buttonLayout->addWidget(m_pagination);
	buttonLayout->addStretch();
	buttonLayout->addSpacing(25);
	buttonLayout->addWidget(m_deleteButton);
	buttonLayout->addSpacing(25);

	m_pagination->SetTotalPageCount(PageCount());
	connect(m_pagination, &Pagination::CurrentPageChanged, this, &MatchHistory::SwitchPage);

	layout->addWidget(m_view);
	layout->addLayout(buttonLayout);

	setLayout(horLayout);

	connect(m_deleteButton, &QPushButton::clicked, [this](bool _)
	{
		const QItemSelection sourceSelection = m_sortFilter->mapSelectionToSource(m_view->selectionModel()->selection());

		if (sourceSelection.empty())
		{
			return;
		}

		int lang = m_services.Get<Config>().Get<ConfigKey::Language>();
		QuestionDialog* dialog = new QuestionDialog(lang, this, GetString(lang, StringTableKey::HISTORY_DELETE_QUESTION));
		if (dialog->Run() == QuestionAnswer::Yes)
		{
			std::vector<int> removedMatches{};

			for (const QItemSelectionRange& s : sourceSelection)
			{
				if (s.isValid() && s.left() == 0)
				{
					int row = s.top();
					removedMatches.emplace_back(row);

					const uint32_t matchId = m_model->GetMatch(row).Id;
					PA_TRYV_OR_ELSE(m_services.Get<DatabaseManager>().DeleteMatch(matchId),
					{
						LOG_ERROR("Failed to delete match from match history: {}", error);
					});
				}
			}

			std::ranges::sort(removedMatches, [](int a, int b){ return a > b; });
			for (int i : removedMatches)
			{
				m_model->DeleteMatch(i);
			}

			m_view->selectionModel()->clearSelection();
			Refresh();
		}
	});

	connect(m_view, &QTableView::doubleClicked, [this](const QModelIndex& index)
	{
		const Client::Match& match = m_model->GetMatch(m_sortFilter->mapToSource(index).row());
		PA_TRY_OR_ELSE(res, ParseMatch(match.Json, MatchContext{}),
		{
			LOG_ERROR("Failed to parse match as JSON: {}", error);
			return;
		});
		emit ReplaySelected(res.Match);
	});

	connect(m_view->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselection)
	{
		m_deleteButton->setEnabled(!selected.empty());
	});
}

void MatchHistory::SwitchPage(int page) const
{
	m_sortFilter->ResetFilter();

	const int fromEntry = std::min(
		page * m_entriesPerPage + m_entriesPerPage - 1,
		std::max(static_cast<int>(m_model->MatchCount()) - 1, 0)
	);
	const int toEntry = page * m_entriesPerPage;

	if (fromEntry == 0 && toEntry == 0)
	{
		m_entryCount->setText("Entries 0 / 0");
		return;
	}

	m_entryCount->setText(std::format("Entries {}-{} / {}", toEntry + 1, fromEntry + 1, m_model->MatchCount()).c_str());

	const size_t from = m_sortFilter->mapToSource(m_sortFilter->index(fromEntry, 0)).row();
	const size_t to = m_sortFilter->mapToSource(m_sortFilter->index(toEntry, 0)).row();

	m_sortFilter->SetFilterRange(
		MatchHistoryModel::GetMatchTime(m_model->GetMatch(from)),
		MatchHistoryModel::GetMatchTime(m_model->GetMatch(to))
	);
}

int MatchHistory::PageCount() const
{
	return std::max(static_cast<int>(std::ceil(m_model->MatchCount() / static_cast<float>(m_entriesPerPage))), 1);
}

void MatchHistory::AddMatch(const Client::Match& match) const
{
	m_model->AddMatch(match);
	Refresh();
}

void MatchHistory::LoadMatches() const
{
	PA_PROFILE_SCOPE();

	LOG_TRACE("Loading MatchHistory...");
	PA_TRY_OR_ELSE(matches, m_services.Get<Client::DatabaseManager>().GetMatches(),
	{
		LOG_ERROR("Failed to get matches from database: {}", error);
		return;
	});
	LOG_TRACE("Loaded MatchHistory");

	m_model->SetMatches(std::move(matches));
	Refresh();
}

void MatchHistory::SetReplaySummary(uint32_t id, const ReplaySummary& summary) const
{
	m_model->SetReplaySummary(id, summary);
}

bool MatchHistory::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == LanguageChangeEvent::RegisteredType())
	{
		int lang = dynamic_cast<LanguageChangeEvent*>(event)->GetLanguage();
		m_deleteButton->setText(GetString(lang, StringTableKey::HISTORY_DELETE));
	}
	return QWidget::eventFilter(watched, event);
}

void MatchHistory::Refresh() const
{
	m_sortFilter->invalidate();
	const int newPageCount = PageCount();
	const int newPage = std::min(m_pagination->CurrentPage(), newPageCount - 1);
	m_pagination->SetTotalPageCount(newPageCount);
	SwitchPage(newPage);
}
