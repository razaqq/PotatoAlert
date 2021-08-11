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


namespace PotatoAlert {

class SettingsWidget : public QWidget
{
	Q_OBJECT
public:
	explicit SettingsWidget(QWidget* parent = nullptr);
	void CheckPath();
private:
	void Init();
	void Load();
	void ConnectSignals();
	void changeEvent(QEvent* event) override;

	QLabel* m_updateLabel = new QLabel();
	QLabel* m_csvLabel = new QLabel();
	QLabel* m_gamePathLabel = new QLabel();
	QLabel* m_replaysFolderLabel = new QLabel();
	QLabel* m_replaysFolderDesc = new QLabel();
	QLabel* m_statsModeLabel = new QLabel();
	QLabel* m_gaLabel = new QLabel();
	QLabel* m_languageLabel = new QLabel();

	SettingsSwitch* m_updates = new SettingsSwitch();
	SettingsSwitch* m_csv = new SettingsSwitch();
	SettingsSwitch* m_overrideReplaysFolder = new SettingsSwitch();
	
	SettingsChoice* m_statsMode;
	SettingsChoice* m_language;

	QPushButton* m_saveButton;
	QPushButton* m_cancelButton;

	// game directory selection
	QLineEdit* m_gamePathEdit = new QLineEdit();
	QToolButton* m_gamePathButton = new QToolButton();
	FolderStatusGui* m_folderStatusGui = new FolderStatusGui(this);

	// manual replays folder
	QLineEdit* m_replaysFolderEdit = new QLineEdit();
	QToolButton* m_replaysFolderButton = new QToolButton();
	std::function<void(bool)> m_toggleReplaysFolderOverride;
signals:
#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"
	void Done();
#pragma clang diagnostic pop
};

}  // namespace PotatoAlert
