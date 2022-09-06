// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/MatchHistory.hpp"
#include "Client/PotatoClient.hpp"
#include "Client/StringTable.hpp"

#include "Core/Log.hpp"
#include "Core/Screenshot.hpp"

#include "Gui/MainWindow.hpp"
#include "Gui/MatchHistory.hpp"
#include "Gui/ReplaySummary.hpp"
#include "Gui/QuestionDialog.hpp"
#include "Gui/StatsWidget/StatsWidget.hpp"
#include "Gui/VerticalMenuBar.hpp"

#include <QDesktopServices>
#include <QMainWindow>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>


using namespace PotatoAlert::Core;
using PotatoAlert::Client::PotatoClient;
using PotatoAlert::Gui::MainWindow;
using PotatoAlert::Gui::MatchHistory;
using PotatoAlert::Gui::QuestionAnswer;
using PotatoAlert::Gui::QuestionDialog;

MainWindow::MainWindow() : QMainWindow()
{
	this->Init();
	this->ConnectSignals();
	PotatoClient::Instance().Init();
}

void MainWindow::Init()
{
	// central widget
	setCentralWidget(m_centralW);

	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->setSpacing(0);

	m_centralLayout->setContentsMargins(0, 0, 0, 0);
	m_centralLayout->setSpacing(0);
	m_centralW->setLayout(m_centralLayout);

	// menubar dock widget
	const bool leftSide = PotatoConfig().Get<ConfigKey::MenuBarLeft>();
	const auto side = leftSide ? Qt::DockWidgetArea::LeftDockWidgetArea : Qt::DockWidgetArea::RightDockWidgetArea;
	addDockWidget(side, m_menuBar);
	connect(m_menuBar, &VerticalMenuBar::dockLocationChanged, [](Qt::DockWidgetArea area)
	{
		PotatoConfig().Set<ConfigKey::MenuBarLeft>(area == Qt::DockWidgetArea::LeftDockWidgetArea);
	});

	// set other tabs invisible
	m_settingsWidget->setVisible(false);
	m_matchHistory->setVisible(false);
	m_replaySummary->setVisible(false);
	m_aboutWidget->setVisible(false);

	m_centralLayout->addWidget(m_statsWidget);
	m_centralLayout->addWidget(m_settingsWidget);
	m_centralLayout->addWidget(m_matchHistory);
	m_centralLayout->addWidget(m_replaySummary);
	m_centralLayout->addWidget(m_aboutWidget);

	// trigger run
	m_settingsWidget->CheckPath();
}

void MainWindow::SwitchTab(MenuEntry i)
{
	QWidget* oldWidget = m_activeWidget;
	switch (i)
	{
	case MenuEntry::Table:
		m_activeWidget = m_statsWidget;
		break;
	case MenuEntry::Settings:
		m_activeWidget = m_settingsWidget;
		break;
	case MenuEntry::MatchHistory:
		m_activeWidget = m_matchHistory;
		break;
	case MenuEntry::Discord:
		QDesktopServices::openUrl(QUrl("https://discord.gg/Ut8t8PA"));
		return;
	case MenuEntry::Screenshot:
		CaptureScreenshot(window());
		return;
	case MenuEntry::CSV:
		QDesktopServices::openUrl(QUrl(Client::MatchHistory::GetDir().absolutePath()));
		return;
	case MenuEntry::Log:
		QDesktopServices::openUrl(QUrl(Log::GetDir().absolutePath()));
		return;
	case MenuEntry::Github:
		QDesktopServices::openUrl(QUrl("https://github.com/razaqq/PotatoAlert"));
		return;
	case MenuEntry::About:
		m_activeWidget = m_aboutWidget;
		break;
	}
	oldWidget->setVisible(false);
	m_activeWidget->setVisible(true);
}

void MainWindow::ConnectSignals()
{
	connect(m_menuBar, &VerticalMenuBar::EntryClicked, this, &MainWindow::SwitchTab);

	connect(&PotatoClient::Instance(), &PotatoClient::StatusReady, m_statsWidget, &StatsWidget::SetStatus);
	connect(&PotatoClient::Instance(), &PotatoClient::MatchReady, m_statsWidget, &StatsWidget::Update);

	connect(&PotatoClient::Instance(), &PotatoClient::MatchHistoryChanged, m_matchHistory, &MatchHistory::UpdateLatest);
	connect(&PotatoClient::Instance(), &PotatoClient::MatchSummaryChanged, m_matchHistory, &MatchHistory::SetSummary);

	connect(m_matchHistory, &MatchHistory::ReplaySelected, [this](const MatchType& match)
	{
		m_statsWidget->Update(match);
		SwitchTab(MenuEntry::Table);
		m_menuBar->SetChecked(MenuEntry::Table);
	});

	connect(m_replaySummary, &ReplaySummary::ReplaySummaryBack, [this]()
	{
		SwitchTab(MenuEntry::MatchHistory);
	});

	// connect(m_matchHistory, &MatchHistory::ReplaySummarySelected, [this](const ReplayParser::ReplaySummary& replaySummary)
	connect(m_matchHistory, &MatchHistory::ReplaySummarySelected, [this](const Client::MatchHistory::Entry& entry)
	{
		m_activeWidget->setVisible(false);
		m_replaySummary->SetReplaySummary(entry);
		m_replaySummary->setVisible(true);
		m_activeWidget = m_replaySummary;
	});

	connect(m_settingsWidget, &SettingsWidget::Done, [this]()
	{
		SwitchTab(MenuEntry::Table);
		m_menuBar->SetChecked(MenuEntry::Table);
	});
}

bool MainWindow::ConfirmUpdate()
{
	/*
	 * TODO: FIX WARNING
	 * QWindowsWindow::setGeometry: Unable to set geometry
	*/

	auto dialog = new QuestionDialog(this, GetString(StringTable::Keys::UPDATE_QUESTION));
	return dialog->Run() == QuestionAnswer::Yes;
}
