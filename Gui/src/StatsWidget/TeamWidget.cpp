// Copyright 2020 <github.com/razaqq>

#include "Client/PotatoClient.hpp"
#include "Client/ServiceProvider.hpp"

#include "Gui/Fonts.hpp"
#include "Gui/StatsWidget/StatsHeader.hpp"
#include "Gui/StatsWidget/StatsTable.hpp"
#include "Gui/StatsWidget/StatsTeamFooter.hpp"
#include "Gui/StatsWidget/TeamWidget.hpp"

#include <QApplication>
#include <QDesktopServices>
#include <QVBoxLayout>
#include <QWidget>

#include <string>


using PotatoAlert::Gui::TeamWidget;

TeamWidget::TeamWidget(Side side, QWidget* parent)
	: QWidget(parent), m_side(side), m_table(new StatsTable())
{
	qApp->installEventFilter(this);

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
			const QUrl url(m_wowsNumbers[static_cast<size_t>(row)]);
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
			if (static_cast<size_t>(col) >= player.size())
			{
				break;
			}

			Client::StatsParser::FieldType field = player[static_cast<size_t>(col)];
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

QRect TeamWidget::GetPlayerColumnRect(QWidget* parent) const
{
	const QRect rectTop = m_table->visualRect(m_table->model()->index(0, 0));
	const QRect rectBottom = m_table->visualRect(m_table->model()->index(m_table->rowCount() - 1, 0));
	const QPoint tl = m_table->viewport()->mapTo(parent, rectTop.topLeft());
	const QPoint br = m_table->viewport()->mapTo(parent, rectBottom.bottomRight());
	return { tl, br };
}

bool TeamWidget::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::ApplicationFontChange)
	{
		UpdateLayoutFont(layout());
	}
	return QWidget::eventFilter(watched, event);
}
