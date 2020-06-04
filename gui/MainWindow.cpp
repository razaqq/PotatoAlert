// Copyright 2020 <github.com/razaqq>

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSizeGrip>
#include <QDockWidget>
#include <QIcon>
#include <QUrl>
#include <QSettings>
#include <QWindow>
#include <QButtonGroup>
#include <QDesktopServices>
#include "../utils/Config.h"
#include "../utils/Logger.h"
#include "../utils/PotatoClient.h"
#include "StatsWidget/StatsWidget.h"
#include "StatsWidget/StatsHeader.h"
#include "MenuBar/VerticalMenuBar.h"
#include "MainWindow.h"


using PotatoAlert::MainWindow;

MainWindow::MainWindow(Config* c, Logger* l, PotatoClient* pc) : QMainWindow()
{
	this->c = c;
	this->l = l;
	this->pc = pc;
	this->init();
	this->connectSignals();
	this->pc->init();
}

MainWindow::~MainWindow()
{
}

void MainWindow::init()
{
	// this->setMouseTracking(true);

	// central widget
	this->setCentralWidget(this->centralW);

	this->layout()->setContentsMargins(0, 0, 0, 0);
	this->layout()->setSpacing(0);
	this->centralLayout->setContentsMargins(0, 0, 0, 0);
	this->centralLayout->setSpacing(0);
	this->centralW->setLayout(centralLayout);

	// menubar dock widget
	QDockWidget* dock = new QDockWidget(this);
	dock->setTitleBarWidget(new QWidget(this));
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	dock->setFeatures(QDockWidget::DockWidgetMovable);
	dock->setMinimumHeight(100);
	dock->setWidget(this->menuBar);
	this->addDockWidget(Qt::LeftDockWidgetArea, dock);

	this->settingsWidget = new SettingsWidget(this, this->c);

	// set other tabs invisible
	this->settingsWidget->setVisible(false);
	this->helpWidget->setVisible(false);
	this->aboutWidget->setVisible(false);

	this->centralLayout->addWidget(this->statsWidget);
	this->centralLayout->addWidget(this->settingsWidget);
	this->centralLayout->addWidget(this->helpWidget);
	this->centralLayout->addWidget(this->aboutWidget);
}

void MainWindow::switchTab(int i)
{
	QWidget* oldWidget = this->activeWidget;
	switch (i)
	{
	case 4:  // log
		QDesktopServices::openUrl(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)));
		return;
	case 5:  // github
		QDesktopServices::openUrl(QUrl("https://github.com/razaqq/PotatoAlert"));
		return;
	case 2:  // discord
		QDesktopServices::openUrl(QUrl("https://discord.gg/Ut8t8PA"));
		return;
	case 0:  // stats table
		this->activeWidget = this->statsWidget;
		break;
	case 1:  // settings
		this->activeWidget = this->settingsWidget;
		break;
	case 3:  // help
		this->activeWidget = this->helpWidget;
		break;
	case 6:  // about
		this->activeWidget = this->aboutWidget;
		break;
	default:
		break;
	}
	oldWidget->setVisible(false);
	this->activeWidget->setVisible(true);
}

void MainWindow::connectSignals()
{
	connect(this->menuBar->btnGroup, &QButtonGroup::idClicked, this, &MainWindow::switchTab);

	connect(this->pc, &PotatoClient::status, this->statsWidget, &StatsWidget::setStatus);
	connect(this->pc, &PotatoClient::teamsReady, this->statsWidget, &StatsWidget::fillTables);
	connect(this->pc, &PotatoClient::avgReady, this->statsWidget, &StatsWidget::setAverages);
	connect(this->pc, &PotatoClient::clansReady, this->statsWidget, &StatsWidget::setClans);
	connect(this->pc, &PotatoClient::wowsNumbersReady, this->statsWidget, &StatsWidget::setWowsNumbers);
}
