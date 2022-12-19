// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Client/DatabaseManager.hpp"
#include "Client/ServiceProvider.hpp"
#include "Client/StatsParser.hpp"

#include "Gui/IconButton.hpp"
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

private:
	const Client::ServiceProvider& m_services;
	QPushButton* m_deleteButton = new QPushButton();
	MatchHistoryView* m_view;
	MatchHistoryModel* m_model;
	MatchHistorySortFilter* m_sortFilter;
	QLabel* m_entryCount = new QLabel();
	Pagination* m_pagination = new Pagination();
	int m_page = 0;
	int m_entriesPerPage = 100;

public:
	explicit MatchHistory(const Client::ServiceProvider& serviceProvider, QWidget* parent = nullptr);

	void SwitchPage(int page) const;
	[[nodiscard]] int PageCount() const;
	void AddMatch(const Client::Match& match) const;
	void SetReplaySummary(uint32_t id, const ReplaySummary& summary) const;
	
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void LoadMatches() const;
	void Refresh() const;

signals:
	void ReplaySelected(const Client::StatsParser::MatchType& match);
	void ReplaySummarySelected(const Client::Match& match);
};

}  // namespace PotatoAlert::Gui
