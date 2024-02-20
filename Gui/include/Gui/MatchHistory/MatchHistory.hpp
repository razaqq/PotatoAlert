// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Client/DatabaseManager.hpp"
#include "Client/ServiceProvider.hpp"
#include "Client/StatsParser.hpp"

#include "Gui/IconButton.hpp"
#include "Gui/MatchHistory/MatchHistoryFilter.hpp"
#include "Gui/MatchHistory/MatchHistoryModel.hpp"
#include "Gui/MatchHistory/MatchHistorySortFilter.hpp"
#include "Gui/MatchHistory/MatchHistoryView.hpp"
#include "Gui/Pagination.hpp"

#include <QLabel>
#include <QPushButton>


namespace PotatoAlert::Gui {

class MatchHistory : public QWidget
{
	Q_OBJECT

public:
	explicit MatchHistory(const Client::ServiceProvider& serviceProvider, QWidget* parent = nullptr);

	void SwitchPage(int page) const;
	[[nodiscard]] int PageCount() const;
	void AddMatch(const Client::Match& match) const;
	void SetReplaySummary(uint32_t id, const ReplaySummary& summary) const;
	
	bool eventFilter(QObject* watched, QEvent* event) override;
	void hideEvent(QHideEvent* event) override;
	void showEvent(QShowEvent* event) override;
	void resizeEvent(QResizeEvent* event) override
	{
		if (m_filter->isVisible())
			m_filter->AdjustPosition();
		QWidget::resizeEvent(event);
	}

private:
	void LoadMatches() const;
	void Refresh() const;

private:
	const Client::ServiceProvider& m_services;
	IconButton* m_deleteButton = new IconButton(":/Delete.svg", ":/DeleteHover.svg", QSize(20, 20), true);
	IconButton* m_filterButton = new IconButton(":/Filter.svg", ":/FilterHover.svg", QSize(20, 20), true);
	MatchHistoryFilter* m_filter = new MatchHistoryFilter(m_filterButton, this);
	MatchHistoryView* m_view;
	MatchHistoryModel* m_model;
	MatchHistorySortFilter* m_sortFilter;
	QLabel* m_entryCount = new QLabel();
	Pagination* m_pagination = new Pagination();
	int m_page = 0;
	static constexpr int EntriesPerPage = 100;

signals:
	void ReplaySelected(const Client::StatsParser::MatchType& match);
	void ReplaySummarySelected(const Client::Match& match);
};

}  // namespace PotatoAlert::Gui
