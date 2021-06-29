// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "Config.hpp"
#include "Logger.hpp"
#include "PotatoClient.hpp"
#include "StatsWidget/StatsWidget.hpp"
#include "SettingsWidget/SettingsWidget.hpp"
#include "AboutWidget.hpp"
#include "MenuBar/VerticalMenuBar.hpp"


namespace PotatoAlert {

class MainWindow : public QMainWindow
{
public:
	explicit MainWindow(PotatoClient* pc);
	bool ConfirmUpdate();
protected:
	void Init();
	PotatoClient* pc;

	void SwitchTab(MenuEntry i);
	void ConnectSignals();

	QWidget* centralW = new QWidget(this);
	QVBoxLayout* centralLayout = new QVBoxLayout;

	VerticalMenuBar* menuBar = new VerticalMenuBar(this);
	StatsWidget* statsWidget = new StatsWidget(this);
	SettingsWidget* settingsWidget{};
	AboutWidget* aboutWidget = new AboutWidget(this);

	QWidget* activeWidget = this->statsWidget;
};

}  // namespace PotatoAlert
