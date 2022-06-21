// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/MatchHistory.hpp"
#include "Client/PotatoClient.hpp"
#include "Client/StringTable.hpp"

#include "Core/Log.hpp"
#include "Core/Screenshot.hpp"

#include "Gui/MainWindow.hpp"
#include "Gui/MatchHistory.hpp"
#include "Gui/MenuBar/VerticalMenuBar.hpp"
#include "Gui/QuestionDialog.hpp"
#include "Gui/StatsWidget/StatsWidget.hpp"

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
	this->setCentralWidget(this->m_centralW);

	this->layout()->setContentsMargins(0, 0, 0, 0);
	this->layout()->setSpacing(0);

	this->m_centralLayout->setContentsMargins(0, 0, 0, 0);
	this->m_centralLayout->setSpacing(0);
	this->m_centralW->setLayout(m_centralLayout);

	// menubar dock widget
	const bool leftSide = PotatoConfig().Get<ConfigKey::MenuBarLeft>();
	const auto side = leftSide ? Qt::DockWidgetArea::LeftDockWidgetArea : Qt::DockWidgetArea::RightDockWidgetArea;
	this->addDockWidget(side, this->m_menuBar);
	connect(this->m_menuBar, &VerticalMenuBar::dockLocationChanged, [](Qt::DockWidgetArea area)
	{
		PotatoConfig().Set<ConfigKey::MenuBarLeft>(area == Qt::DockWidgetArea::LeftDockWidgetArea);
	});

	// set other tabs invisible
	this->m_settingsWidget->setVisible(false);
	this->m_matchHistory->setVisible(false);
	this->m_aboutWidget->setVisible(false);

	this->m_centralLayout->addWidget(this->m_statsWidget);
	this->m_centralLayout->addWidget(this->m_settingsWidget);
	this->m_centralLayout->addWidget(this->m_matchHistory);
	this->m_centralLayout->addWidget(this->m_aboutWidget);

	// trigger run
	this->m_settingsWidget->CheckPath();
}

void MainWindow::SwitchTab(MenuEntry i)
{
	QWidget* oldWidget = this->m_activeWidget;
	switch (i)
	{
	case MenuEntry::Table:
		this->m_activeWidget = this->m_statsWidget;
		break;
	case MenuEntry::Settings:
		this->m_activeWidget = this->m_settingsWidget;
		break;
	case MenuEntry::MatchHistory:
		this->m_activeWidget = this->m_matchHistory;
		break;
	case MenuEntry::Discord:
		QDesktopServices::openUrl(QUrl("https://discord.gg/Ut8t8PA"));
		return;
	case MenuEntry::Screenshot:
		CaptureScreenshot(this->window());
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
		this->m_activeWidget = this->m_aboutWidget;
		break;
	}
	oldWidget->setVisible(false);
	this->m_activeWidget->setVisible(true);
}

void MainWindow::ConnectSignals()
{
	connect(this->m_menuBar, &VerticalMenuBar::EntryClicked, this, &MainWindow::SwitchTab);

	connect(&PotatoClient::Instance(), &PotatoClient::StatusReady, this->m_statsWidget, &StatsWidget::SetStatus);
	connect(&PotatoClient::Instance(), &PotatoClient::MatchReady, this->m_statsWidget, &StatsWidget::Update);

	connect(&PotatoClient::Instance(), &PotatoClient::MatchHistoryChanged, this->m_matchHistory, &MatchHistory::UpdateLatest);
	connect(&PotatoClient::Instance(), &PotatoClient::MatchSummaryChanged, this->m_matchHistory, &MatchHistory::SetSummary);

	connect(this->m_matchHistory, &MatchHistory::ReplaySelected, [this](const Match& match)
	{
		this->m_statsWidget->Update(match);
		this->SwitchTab(MenuEntry::Table);
		this->m_menuBar->SetChecked(MenuEntry::Table);
	});

	connect(this->m_settingsWidget, &SettingsWidget::Done, [this]()
	{
		this->SwitchTab(MenuEntry::Table);
		this->m_menuBar->SetChecked(MenuEntry::Table);
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
