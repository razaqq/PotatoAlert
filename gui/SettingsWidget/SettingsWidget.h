// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QWidget>
#include <QComboBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QToolButton>
#include <QPushButton>
#include "SettingsSwitch.h"
#include "SettingsChoice.h"
#include "FolderStatus.h"
#include "PotatoClient.h"
#include "Config.h"
#include "Logger.h"


namespace PotatoAlert {

class SettingsWidget : public QWidget
{
public:
	SettingsWidget(QWidget* parent, Config* c, Logger* h, PotatoClient* pc);
private:
	void init();
	void load();
	void connectSignals();
    void checkPath();

	Config* config;
	Logger* logger;
	PotatoClient* pc;

	SettingsSwitch* updates = new SettingsSwitch;
	SettingsSwitch* centralApi = new SettingsSwitch;
	SettingsSwitch* googleAnalytics = new SettingsSwitch;
	
	SettingsChoice* statsMode;

	QPushButton* saveButton;
	QPushButton* cancelButton;

	// game directory selection
	QLineEdit* gamePathEdit;
	QToolButton* gamePathButton;
	FolderStatus* folderStatusGui;
};

}  // namespace PotatoAlert
