// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/PotatoClient.hpp"
#include "Client/StatsParser.hpp"

#include "Gui/StatsWidget/TeamWidget.hpp"

#include <QBoxLayout>
#include <QWidget>


namespace PotatoAlert::Gui {

using PotatoAlert::Client::StatsParser::MatchType;

class StatsWidget : public QWidget
{
private:
	const Client::ServiceProvider& m_services;

	TeamWidget* m_team1 = new TeamWidget(TeamWidget::Side::Friendly, m_services);
	TeamWidget* m_team2 = new TeamWidget(TeamWidget::Side::Enemy, m_services);

	QBoxLayout* m_tableLayout;
	QVBoxLayout* m_layout = new QVBoxLayout();

public:
	explicit StatsWidget(const Client::ServiceProvider& serviceProvider, QWidget* parent = nullptr);

	void Update(const MatchType& match) const;
	void SetStatus(Client::Status status, std::string_view statusText) const;
	void UpdateTableLayout();
	void SetShowKarma(bool showKarma) const
	{
		m_team1->SetShowKarma(showKarma);
		m_team2->SetShowKarma(showKarma);
	}

private:
	void AddTables();
	void RemoveTables() const;
	void Init();
};

}  // namespace PotatoAlert::Gui
