// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "Config.h"
#include "Logger.h"
#include "PotatoClient.h"
#include "StatsWidget/StatsWidget.h"
#include "SettingsWidget/SettingsWidget.h"
#include "AboutWidget.h"
#include "MenuBar/VerticalMenuBar.h"


namespace PotatoAlert {

class MainWindow : public QMainWindow
{
public:
	explicit MainWindow(PotatoClient* pc);
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
