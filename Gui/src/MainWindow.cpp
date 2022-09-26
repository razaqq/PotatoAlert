// Copyright 2020 <github.com/razaqq>

#include "Client/AppDirectories.hpp"
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


using namespace PotatoAlert::Client::StringTable;
using PotatoAlert::Client::ConfigKey;
using PotatoAlert::Client::PotatoClient;
using PotatoAlert::Gui::MainWindow;
using PotatoAlert::Gui::MatchHistory;
using PotatoAlert::Gui::QuestionAnswer;
using PotatoAlert::Gui::QuestionDialog;

MainWindow::MainWindow(const Client::ServiceProvider& serviceProvider)
	: QMainWindow(), m_services(serviceProvider)
{
	Init();
	ConnectSignals();
	serviceProvider.Get<PotatoClient>().Init();
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

	Config& config = m_services.Get<Config>();

	// menubar dock widget
	const bool leftSide = config.Get<ConfigKey::MenuBarLeft>();
	const auto side = leftSide ? Qt::DockWidgetArea::LeftDockWidgetArea : Qt::DockWidgetArea::RightDockWidgetArea;
	addDockWidget(side, m_menuBar);
	connect(m_menuBar, &VerticalMenuBar::dockLocationChanged, [&config](Qt::DockWidgetArea area)
	{
		config.Set<ConfigKey::MenuBarLeft>(area == Qt::DockWidgetArea::LeftDockWidgetArea);
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
		{
			m_activeWidget = m_statsWidget;
			break;
		}
		case MenuEntry::Settings:
		{
			m_activeWidget = m_settingsWidget;
			break;
		}
		case MenuEntry::MatchHistory:
		{
			m_activeWidget = m_matchHistory;
			break;
		}
		case MenuEntry::Discord:
		{
			QDesktopServices::openUrl(QUrl("https://discord.gg/Ut8t8PA"));
			return;
		}
		case MenuEntry::Screenshot:
		{
			const auto screenshotDir = m_services.Get<Client::AppDirectories>().ScreenshotsDir;
			Core::CaptureScreenshot(window(), screenshotDir);
			QDesktopServices::openUrl(QUrl::fromLocalFile(screenshotDir.string().c_str()));
			return;
		}
		case MenuEntry::CSV:
		{
			QDesktopServices::openUrl(QUrl::fromLocalFile(m_services.Get<Client::AppDirectories>().MatchesDir.string().c_str()));
			return;
		}
		case MenuEntry::Log:
		{
			QDesktopServices::openUrl(QUrl::fromLocalFile(m_services.Get<Client::AppDirectories>().AppDir.string().c_str()));
			return;
		}
		case MenuEntry::Github:
		{
			QDesktopServices::openUrl(QUrl("https://github.com/razaqq/PotatoAlert"));
			return;
		}
		case MenuEntry::About:
		{
			m_activeWidget = m_aboutWidget;
			break;
		}
	}
	oldWidget->setVisible(false);
	m_activeWidget->setVisible(true);
}

void MainWindow::ConnectSignals()
{
	connect(m_menuBar, &VerticalMenuBar::EntryClicked, this, &MainWindow::SwitchTab);

	const PotatoClient& potatoClient = m_services.Get<PotatoClient>();

	connect(&potatoClient, &PotatoClient::StatusReady, m_statsWidget, &StatsWidget::SetStatus);
	connect(&potatoClient, &PotatoClient::MatchReady, m_statsWidget, &StatsWidget::Update);

	connect(&potatoClient, &PotatoClient::MatchHistoryChanged, m_matchHistory, &MatchHistory::UpdateLatest);
	connect(&potatoClient, &PotatoClient::MatchSummaryChanged, m_matchHistory, &MatchHistory::SetSummary);

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

	int lang = m_services.Get<Config>().Get<ConfigKey::Language>();
	auto dialog = new QuestionDialog(lang, this, GetString(lang, StringTableKey::UPDATE_QUESTION));
	return dialog->Run() == QuestionAnswer::Yes;
}
