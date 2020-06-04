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
#include "../../utils/Config.h"


namespace PotatoAlert {

class SettingsWidget : public QWidget
{
public:
	SettingsWidget(QWidget* parent, Config* c);
private:
	void init();
	void save();
	void load();
	void connectSignals();

	Config* config;

	SettingsSwitch* updates = new SettingsSwitch;
	SettingsSwitch* centralApi = new SettingsSwitch;
	SettingsSwitch* googleAnalytics = new SettingsSwitch;
	
	SettingsChoice* statsMode;

	QPushButton* saveButton;
	QPushButton* cancelButton;

	// game directory selection
	QFileDialog* gamePathDialog;
	QLineEdit* gamePathEdit;
	QToolButton* gamePathButton;
};

}  // namespace PotatoAlert
