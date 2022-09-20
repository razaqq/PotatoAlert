// Copyright 2020 <github.com/razaqq>
#pragma once

#include "FolderStatus.hpp"
#include "SettingsChoice.hpp"
#include "SettingsSwitch.hpp"

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
	void Init();
	void Load() const;
	void ConnectSignals();
	void changeEvent(QEvent* event) override;

	QLabel* m_updateLabel = new QLabel();
	QLabel* m_minimizeTrayLabel = new QLabel();
	QLabel* m_matchHistoryLabel = new QLabel();
	QLabel* m_gamePathLabel = new QLabel();
	QLabel* m_statsModeLabel = new QLabel();
	QLabel* m_teamDamageModeLabel = new QLabel();
	QLabel* m_teamWinRateModeLabel = new QLabel();
	QLabel* m_languageLabel = new QLabel();

	SettingsSwitch* m_updates = new SettingsSwitch();
	SettingsSwitch* m_minimizeTray = new SettingsSwitch();
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
	bool eventFilter(QObject* watched, QEvent* event) override;

signals:
	void Done();
};

}  // namespace PotatoAlert::Gui
