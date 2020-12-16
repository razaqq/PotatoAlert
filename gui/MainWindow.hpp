// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "Config.hpp"
#include "Logger.hpp"
#include "Updater.hpp"
#include "PotatoClient.hpp"
#include "StatsWidget/StatsWidget.h"
#include "SettingsWidget/SettingsWidget.hpp"
#include "AboutWidget.hpp"
#include "MenuBar/VerticalMenuBar.hpp"


namespace PotatoAlert {

class MainWindow : public QMainWindow
{
public:
	explicit MainWindow(PotatoClient* pc);
	int confirmUpdate();
	void updateProgress(qint64 bytesReceived, qint64 bytesTotal);
	void startUpdate(Updater* updater);
protected:
	void init();
	PotatoClient* pc;

	void switchTab(int i);
	void connectSignals();

	QWidget* centralW = new QWidget(this);
	QVBoxLayout* centralLayout = new QVBoxLayout;

	VerticalMenuBar* menuBar = new VerticalMenuBar(this);
	StatsWidget* statsWidget = new StatsWidget(this);
	SettingsWidget* settingsWidget{};
	AboutWidget* aboutWidget = new AboutWidget(this);

	QWidget* activeWidget = this->statsWidget;
};

}  // namespace PotatoAlert
