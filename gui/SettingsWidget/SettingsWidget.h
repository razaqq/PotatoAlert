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
#include "SettingsSwitch.h"
#include "SettingsChoice.h"
#include "FolderStatus.h"
#include "PotatoClient.h"
#include "Config.h"
#include "Logger.h"


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
    void changeEvent(QEvent* event);

	PotatoClient* pc;

    QLabel* updateLabel = new QLabel;
    QLabel* csvLabel = new QLabel;
    QLabel* gamePathLabel = new QLabel;
    QLabel* statsModeLabel = new QLabel;
    QLabel* gaLabel = new QLabel;
    QLabel* languageLabel = new QLabel;

	SettingsSwitch* updates = new SettingsSwitch;
	SettingsSwitch* csv = new SettingsSwitch;
	SettingsSwitch* googleAnalytics = new SettingsSwitch;
	
	SettingsChoice* statsMode;
    SettingsChoice* language;

	QPushButton* saveButton;
	QPushButton* cancelButton;

	// game directory selection
	QLineEdit* gamePathEdit;
	QToolButton* gamePathButton;
	FolderStatus* folderStatusGui;
signals:
    void done();
};

}  // namespace PotatoAlert
