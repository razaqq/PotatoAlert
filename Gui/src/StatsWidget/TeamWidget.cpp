// Copyright 2020 <github.com/razaqq>

#include "Client/PotatoClient.hpp"
#include "Client/ServiceProvider.hpp"

#include "Gui/LanguageChangeEvent.hpp"
#include "Gui/StatsWidget/StatsHeader.hpp"
#include "Gui/StatsWidget/StatsTable.hpp"
#include "Gui/StatsWidget/StatsTeamFooter.hpp"
#include "Gui/StatsWidget/TeamWidget.hpp"

#include <QDesktopServices>
#include <QVBoxLayout>
#include <QWidget>

#include <string>


using PotatoAlert::Gui::TeamWidget;

TeamWidget::TeamWidget(Side side, const Client::ServiceProvider& serviceProvider, QWidget* parent)
	: QWidget(parent), m_side(side), m_table(new StatsTable(serviceProvider.Get<Client::Config>().Get<Client::ConfigKey::ShowKarma>())), m_services(serviceProvider)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	if (side == Side::Friendly)
	{
		m_header = new StatsHeaderFriendly();
	}
	else
	{
		m_header = new StatsHeaderEnemy();
	}

	layout->addWidget(m_header);
	layout->addWidget(m_table);
	layout->addWidget(m_footer);

	setLayout(layout);

	connect(m_table, &StatsTable::cellDoubleClicked, [this](int row, [[maybe_unused]] int column)
	{
		if (static_cast<size_t>(row) < m_wowsNumbers.size())
		{
			const QUrl url(m_wowsNumbers[row]);
			if (url.isValid())
				QDesktopServices::openUrl(url);
		}
	});
}

void TeamWidget::Update(const Team& team)
{
	m_wowsNumbers = team.WowsNumbers;

	m_table->clearContents();

	int row = 0;
	for (auto& player : team.Table)
	{
		for (int col = 0; col < m_table->columnCount(); col++)
		{
			if (col >= player.size())
			{
				break;
			}

			Client::StatsParser::FieldType field = player[col];
			if (std::holds_alternative<QTableWidgetItem*>(field))
				m_table->setItem(row, col, std::get<QTableWidgetItem*>(field));
			if (std::holds_alternative<QLabel*>(field))
				m_table->setCellWidget(row, col, std::get<QLabel*>(field));
			if (std::holds_alternative<QWidget*>(field))
				m_table->setCellWidget(row, col, std::get<QWidget*>(field));
		}
		row++;
	}
	m_table->resizeColumnToContents(1);

	m_footer->Update(team);
}

void TeamWidget::SetStatus(Client::Status status, std::string_view text) const
{
	if (m_side == Side::Friendly)
	{
		dynamic_cast<StatsHeaderFriendly*>(m_header)->SetStatus(status, text);
	}
}

void TeamWidget::SetShowKarma(bool showKarma) const
{
	m_table->SetShowKarma(showKarma);
}
