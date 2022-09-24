// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Gui/SettingsWidget/FolderStatus.hpp"
#include "Gui/SettingsWidget/SettingsChoice.hpp"
#include "Gui/SettingsWidget/SettingsSwitch.hpp"

#include <QComboBox>
#include <QEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QWidget>


namespace PotatoAlert::Gui {

class SettingsWidget : public QWidget
{
	Q_OBJECT

private:
	const Client::ServiceProvider& m_services;

	QLabel* m_updateLabel = new QLabel();
	QLabel* m_minimizeTrayLabel = new QLabel();
	QLabel* m_saveMatchCsvLabel = new QLabel();
	QLabel* m_matchHistoryLabel = new QLabel();
	QLabel* m_gamePathLabel = new QLabel();
	QLabel* m_statsModeLabel = new QLabel();
	QLabel* m_teamDamageModeLabel = new QLabel();
	QLabel* m_teamWinRateModeLabel = new QLabel();
	QLabel* m_languageLabel = new QLabel();

	SettingsSwitch* m_updates = new SettingsSwitch();
	SettingsSwitch* m_minimizeTray = new SettingsSwitch();
	SettingsSwitch* m_saveMatchCsv = new SettingsSwitch();
	SettingsSwitch* m_matchHistory = new SettingsSwitch();
	
	SettingsChoice* m_statsMode;
	SettingsChoice* m_teamDamageMode;
	SettingsChoice* m_teamWinRateMode;
	QComboBox* m_language = new QComboBox();

	QPushButton* m_saveButton;
	QPushButton* m_cancelButton;

	// game directory selection
	QLineEdit* m_gamePathEdit = new QLineEdit();
	QToolButton* m_gamePathButton = new QToolButton();
	FolderStatus* m_folderStatusGui = new FolderStatus(this);

	bool m_forceRun = false;

public:
	explicit SettingsWidget(const Client::ServiceProvider& serviceProvider, QWidget* parent = nullptr);
	void CheckPath() const;
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void Init();
	void Load() const;
	void ConnectSignals();

signals:
	void Done();
};

}  // namespace PotatoAlert::Gui
