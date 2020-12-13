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
#include "Config.h"
#include "Logger.h"
#include "PotatoClient.h"
#include "StatsWidget/StatsWidget.h"
#include "StatsWidget/StatsHeader.hpp"
#include "MenuBar/VerticalMenuBar.hpp"
#include "MainWindow.hpp"


using PotatoAlert::MainWindow;

MainWindow::MainWindow(PotatoClient* pc) : QMainWindow()
{
	this->pc = pc;
	this->init();
	this->connectSignals();
	this->pc->init();
}

void MainWindow::init()
{
	// central widget
	this->setCentralWidget(this->centralW);

	this->layout()->setContentsMargins(0, 0, 0, 0);
	this->layout()->setSpacing(0);
	this->centralLayout->setContentsMargins(0, 0, 0, 0);
	this->centralLayout->setSpacing(0);
	this->centralW->setLayout(centralLayout);

	// menubar dock widget
	auto dock = new QDockWidget(this);
	dock->setTitleBarWidget(new QWidget(this));
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	dock->setFeatures(QDockWidget::DockWidgetMovable);
	dock->setMinimumHeight(100);
	dock->setWidget(this->menuBar);
	this->addDockWidget(Qt::LeftDockWidgetArea, dock);

	this->settingsWidget = new SettingsWidget(this, this->pc);

	// set other tabs invisible
	this->settingsWidget->setVisible(false);
	this->aboutWidget->setVisible(false);

	this->centralLayout->addWidget(this->statsWidget);
	this->centralLayout->addWidget(this->settingsWidget);
	this->centralLayout->addWidget(this->aboutWidget);
}

void MainWindow::switchTab(int i)
{
	QWidget* oldWidget = this->activeWidget;
	switch (i)
	{
	case 0:  // stats table
        this->activeWidget = this->statsWidget;
        break;
	case 1:  // settings
		this->activeWidget = this->settingsWidget;
		break;
	case 2:  // discord
        QDesktopServices::openUrl(QUrl("https://discord.gg/Ut8t8PA"));
        return;
    case 3:  // log
        QDesktopServices::openUrl(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/PotatoAlert"));
        return;
    case 4:  // github
        QDesktopServices::openUrl(QUrl("https://github.com/razaqq/PotatoAlert"));
        return;
	case 5:  // about
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

	connect(this->settingsWidget, &SettingsWidget::done,[this](){ this->switchTab(0); this->menuBar->btnGroup->button(0)->setChecked(true); });
}
