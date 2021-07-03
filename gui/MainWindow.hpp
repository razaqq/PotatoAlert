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
	MainWindow();
	bool ConfirmUpdate();
protected:
	void Init();

	void SwitchTab(MenuEntry i);
	void ConnectSignals();
	void showEvent(QShowEvent* event) override;

	QWidget* m_centralW = new QWidget(this);
	QVBoxLayout* m_centralLayout = new QVBoxLayout();

	VerticalMenuBar* m_menuBar = new VerticalMenuBar(this);
	StatsWidget* m_statsWidget = new StatsWidget(this);
	SettingsWidget* m_settingsWidget{};
	AboutWidget* m_aboutWidget = new AboutWidget(this);

	QWidget* m_activeWidget = this->m_statsWidget;
};

}  // namespace PotatoAlert
