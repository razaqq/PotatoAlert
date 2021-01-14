// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>
#include <QEvent>
#include <QComboBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QToolButton>
#include <QPushButton>
#include "SettingsSwitch.hpp"
#include "SettingsChoice.hpp"
#include "FolderStatus.hpp"
#include "PotatoClient.hpp"
#include "Config.hpp"
#include "Logger.hpp"


namespace PotatoAlert {

class SettingsWidget : public QWidget
{
	Q_OBJECT
public:
	SettingsWidget(QWidget* parent, PotatoClient* pc);
private:
	void init();
	void load();
	void connectSignals();
	void checkPath();
	void changeEvent(QEvent* event) override;

	PotatoClient* pc;

	QLabel* updateLabel = new QLabel();
	QLabel* csvLabel = new QLabel();
	QLabel* gamePathLabel = new QLabel();
	QLabel* replaysFolderLabel = new QLabel();
	QLabel* replaysFolderDesc = new QLabel();
	QLabel* statsModeLabel = new QLabel();
	QLabel* gaLabel = new QLabel();
	QLabel* languageLabel = new QLabel();

	SettingsSwitch* updates = new SettingsSwitch();
	SettingsSwitch* csv = new SettingsSwitch();
	SettingsSwitch* overrideReplaysFolder = new SettingsSwitch();
	
	SettingsChoice* statsMode;
	SettingsChoice* language;

	QPushButton* saveButton;
	QPushButton* cancelButton;

	// game directory selection
	QLineEdit* gamePathEdit = new QLineEdit();
	QToolButton* gamePathButton = new QToolButton();
	FolderStatus* folderStatusGui;

	// manual replays folder
	QLineEdit* replaysFolderEdit = new QLineEdit();
	QToolButton* replaysFolderButton = new QToolButton();
	std::function<void(bool)> toggleReplaysFolderOverride;
signals:
	void done();
};

}  // namespace PotatoAlert
