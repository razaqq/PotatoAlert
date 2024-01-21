// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/ServiceProvider.hpp"

#include "Gui/AboutWidget.hpp"
#include "Gui/Events.hpp"
#include "Gui/MatchHistory/MatchHistory.hpp"
#include "Gui/ReplaySummary.hpp"
#include "Gui/SettingsWidget/SettingsWidget.hpp"
#include "Gui/StatsWidget/StatsWidget.hpp"
#include "Gui/VerticalMenuBar.hpp"

#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>


namespace PotatoAlert::Gui {

class MainWindow : public QMainWindow
{

public:
	explicit MainWindow(const Client::ServiceProvider& serviceProvider);
	bool ConfirmUpdate();

private:
	void Init();
	void SwitchTab(MenuEntry i);
	void ConnectSignals();

protected:
	bool event(QEvent* e) override;

private:
	const Client::ServiceProvider& m_services;

	QWidget* m_centralW = new QWidget(this);
	QVBoxLayout* m_centralLayout = new QVBoxLayout();

	VerticalMenuBar* m_menuBar = new VerticalMenuBar(this);
	StatsWidget* m_statsWidget = new StatsWidget(m_services, this);
	SettingsWidget* m_settingsWidget = new SettingsWidget(m_services, this);
	MatchHistory* m_matchHistory = new MatchHistory(m_services, this);
	ReplaySummary* m_replaySummary = new ReplaySummary(m_services, this);
	AboutWidget* m_aboutWidget = new AboutWidget(this);

	QWidget* m_activeWidget = m_statsWidget;
};

}  // namespace PotatoAlert::Gui
