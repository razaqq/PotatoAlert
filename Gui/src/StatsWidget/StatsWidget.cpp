// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/PotatoClient.hpp"
#include "Client/ServiceProvider.hpp"
#include "Client/StatsParser.hpp"

#include "Gui/StatsWidget/StatsHeader.hpp"
#include "Gui/StatsWidget/StatsWidget.hpp"

#include <QDesktopServices>
#include <QTableWidgetItem>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>


using PotatoAlert::Client::Config;
using PotatoAlert::Client::ConfigKey;
using PotatoAlert::Client::ServiceProvider;
using PotatoAlert::Gui::StatsWidget;

StatsWidget::StatsWidget(const ServiceProvider& serviceProvider, QWidget* parent) : QWidget(parent), m_services(serviceProvider)
{
	Init();
}

void StatsWidget::Init()
{
	// setStyleSheet("border: 1px solid red;");

	m_layout->setContentsMargins(0, 0, 0, 10);
	m_layout->setSpacing(0);

	AddTables();

	setLayout(m_layout);
}

void StatsWidget::Update(const MatchType& match) const
{
	m_team1->Update(match.Team1);
	m_team2->Update(match.Team2);
}

void StatsWidget::SetStatus(Client::Status status, std::string_view statusText) const
{
	m_team1->SetStatus(status, statusText);
}

void StatsWidget::AddTables()
{
	switch (m_services.Get<Config>().Get<ConfigKey::TableLayout>())
	{
		case Client::TableLayout::Horizontal:
		{
			setMinimumSize(1200, 323);
			m_tableLayout = new QHBoxLayout();
			m_tableLayout->setContentsMargins(10, 0, 10, 0);
			m_tableLayout->setSpacing(10);

			m_tableLayout->addWidget(m_team1);
			m_tableLayout->addWidget(m_team2);

			m_layout->addLayout(m_tableLayout);
			break;
		}
		case Client::TableLayout::Vertical:
		{
			setMinimumSize(600, 646);
			m_tableLayout = new QVBoxLayout();
			m_tableLayout->setContentsMargins(10, 0, 10, 0);
			m_tableLayout->setSpacing(10);

			m_tableLayout->addWidget(m_team1);
			m_tableLayout->addWidget(m_team2);

			m_layout->addLayout(m_tableLayout);
			break;
		}
	}
}

void StatsWidget::RemoveTables() const
{
	m_tableLayout->removeWidget(m_team1);
	m_tableLayout->removeWidget(m_team2);
	m_layout->removeItem(m_tableLayout);
}

void StatsWidget::UpdateTableLayout()
{
	RemoveTables();
	AddTables();
}
