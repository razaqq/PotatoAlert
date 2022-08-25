// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Gui/AboutWidget.hpp"
#include "Gui/MatchHistory.hpp"
#include "Gui/MenuBar/VerticalMenuBar.hpp"
#include "Gui/SettingsWidget/SettingsWidget.hpp"
#include "Gui/StatsWidget/StatsWidget.hpp"

#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>


namespace PotatoAlert::Gui {

class MainWindow : public QMainWindow
{
public:
	MainWindow();
	bool ConfirmUpdate();

private:
	void Init();

	void SwitchTab(MenuEntry i);
	void ConnectSignals();

	QWidget* m_centralW = new QWidget(this);
	QVBoxLayout* m_centralLayout = new QVBoxLayout();

	VerticalMenuBar* m_menuBar = new VerticalMenuBar(this);
	StatsWidget* m_statsWidget = new StatsWidget(this);
	SettingsWidget* m_settingsWidget = new SettingsWidget(this);
	MatchHistory* m_matchHistory = new MatchHistory(this);
	AboutWidget* m_aboutWidget = new AboutWidget(this);

	QWidget* m_activeWidget = this->m_statsWidget;
};

}  // namespace PotatoAlert::Gui
