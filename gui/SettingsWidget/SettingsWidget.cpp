// Copyright 2020 <github.com/razaqq>

#include <QIcon>
#include <QSize>
#include <QFrame>
#include <QPixmap>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QLineEdit>
#include <QFileDialog>
#include <QToolButton>
#include <QPushButton>
#include <QApplication>
#include "SettingsWidget.h"
#include "Game.h"
#include "Config.h"
#include "Logger.h"
#include "PotatoClient.h"
#include "SettingsSwitch.h"
#include "SettingsChoice.h"
#include "HorizontalLine.h"
#include "FolderStatus.h"
#include "StringTable.h"


const int LABEL_WIDTH = 220;
const int ROW_HEIGHT = 20;
const int ROW_WIDTH = 520;

using PotatoAlert::SettingsWidget;
using PotatoAlert::SettingsChoice;
using PotatoAlert::Game;

SettingsWidget::SettingsWidget(QWidget* parent, PotatoClient* pc) : QWidget(parent)
{
	this->pc = pc;
	this->init();
	this->connectSignals();
	this->checkPath();
}

void SettingsWidget::init()
{
	this->folderStatusGui = new FolderStatus(this);

    auto horLayout = new QHBoxLayout;
	horLayout->setContentsMargins(10, 10, 10, 10);
	horLayout->setSpacing(0);
    auto centralWidget = new QWidget(this);
	centralWidget->setObjectName("settingsWidget");
	horLayout->addStretch();
	horLayout->addWidget(centralWidget);
	horLayout->addStretch();

	QFont labelFont("Helvetica Neue", 13, QFont::Bold);
	labelFont.setStyleStrategy(QFont::PreferAntialias);

    auto layout = new QVBoxLayout;
	
	/* UPDATE NOTIFICATIONS */
    auto updateLayout = new QHBoxLayout;
	this->updateLabel->setFont(labelFont);
    this->updateLabel->setFixedWidth(LABEL_WIDTH);
	updateLayout->addWidget(this->updateLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	updateLayout->addWidget(this->updates, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(updateLayout);

	layout->addWidget(new HorizontalLine(centralWidget));

	/* SELECTOR FOR GAME FOLDER */
    auto gamePathLayout = new QHBoxLayout;
    this->gamePathLabel->setFont(labelFont);
    this->gamePathLabel->setFixedWidth(LABEL_WIDTH);
	gamePathLayout->addWidget(this->gamePathLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	gamePathLayout->addStretch();

	this->gamePathEdit = new QLineEdit(centralWidget);
	this->gamePathEdit->setFixedSize(278, ROW_HEIGHT);
	this->gamePathEdit->setReadOnly(true);
	this->gamePathEdit->setFocusPolicy(Qt::NoFocus);
	gamePathLayout->addWidget(this->gamePathEdit, 0, Qt::AlignVCenter | Qt::AlignRight);

	this->gamePathButton = new QToolButton(this);
	this->gamePathButton->setIcon(QIcon(QPixmap(":/folder.svg")));
	this->gamePathButton->setIconSize(QSize(ROW_HEIGHT, ROW_HEIGHT));
	this->gamePathButton->setCursor(Qt::PointingHandCursor);
	gamePathLayout->addWidget(this->gamePathButton, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(gamePathLayout);

	this->folderStatusGui = new FolderStatus(this);
    layout->addWidget(this->folderStatusGui);

	layout->addWidget(new HorizontalLine(centralWidget));

	/* DISPLAYED STATS MODE */
    auto statsModeLayout = new QHBoxLayout;
    this->statsModeLabel->setFont(labelFont);
    this->statsModeLabel->setFixedWidth(LABEL_WIDTH);
	statsModeLayout->addWidget(this->statsModeLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	this->statsMode = new SettingsChoice(this, std::vector<QString>{"current mode", "pvp", "ranked", "clan"});  // TODO: localize
	statsModeLayout->addWidget(this->statsMode, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(statsModeLayout);

	layout->addWidget(new HorizontalLine(centralWidget));

	/* GOOGLE ANALYTICS */
    auto gaLayout = new QHBoxLayout;
    this->gaLabel->setFont(labelFont);
    this->gaLabel->setFixedWidth(LABEL_WIDTH);
	gaLayout->addWidget(this->gaLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	this->googleAnalytics->setDisabled(true);
	gaLayout->addWidget(this->googleAnalytics, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(gaLayout);

    layout->addWidget(new HorizontalLine(centralWidget));

	/* LANGUAGE */
    auto languageLayout = new QHBoxLayout;
    this->languageLabel->setFont(labelFont);
    this->languageLabel->setFixedWidth(LABEL_WIDTH);
    languageLayout->addWidget(this->languageLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

    std::vector<QString> langs;
    for (auto& lang : Languages)
    {
        langs.push_back(QString::fromStdString(std::string(lang)));
    }
    this->language = new SettingsChoice(this, langs);
    languageLayout->addWidget(this->language, 0, Qt::AlignVCenter | Qt::AlignRight);
    layout->addLayout(languageLayout);

    layout->addWidget(new HorizontalLine(centralWidget));

    /* CSV OUTPUT */
    auto csvLayout = new QHBoxLayout;
    this->csvLabel->setFixedWidth(LABEL_WIDTH);
    this->csvLabel->setFont(labelFont);
    this->csvLabel->setFixedWidth(LABEL_WIDTH);
    csvLayout->addWidget(this->csvLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
    csvLayout->addWidget(this->csv, 0, Qt::AlignVCenter | Qt::AlignRight);
    layout->addLayout(csvLayout);

	layout->addStretch();

	/* SAVE & CANCEL BUTTON */
	// TODO: rework buttons
    auto confirmLayout = new QHBoxLayout;
	this->saveButton = new QPushButton;
	this->saveButton->setFixedWidth(100);
	this->cancelButton = new QPushButton;
    this->cancelButton->setFixedWidth(100);
    confirmLayout->addStretch();
	confirmLayout->addWidget(this->saveButton);
	confirmLayout->addWidget(this->cancelButton);
    confirmLayout->addStretch();
	layout->addLayout(confirmLayout);

	centralWidget->setLayout(layout);
	this->setLayout(horLayout);

	this->load();
}

void SettingsWidget::load()
{
	this->updates->setChecked(PotatoConfig().get<bool>("update_notifications"));
	this->googleAnalytics->setChecked(PotatoConfig().get<bool>("use_ga"));
	this->gamePathEdit->setText(QString::fromStdString(PotatoConfig().get<std::string>("game_folder")));
	this->statsMode->btnGroup->button(PotatoConfig().get<int>("stats_mode"))->setChecked(true);
    this->language->btnGroup->button(PotatoConfig().get<int>("language"))->setChecked(true);
    this->csv->setChecked(PotatoConfig().get<bool>("save_csv"));
}

void SettingsWidget::connectSignals()
{
	connect(this->saveButton, &QPushButton::clicked, [this]() { PotatoConfig().save(); emit this->done(); });
	connect(this->cancelButton, &QPushButton::clicked, [this]() {
	    PotatoConfig().load();
	    this->load();
	    this->checkPath();
        QEvent event(QEvent::LanguageChange);
        QApplication::sendEvent(this->window(), &event);
        emit this->done();
	});
	connect(this->updates, &SettingsSwitch::clicked, [](bool checked) { PotatoConfig().set<bool>("update_notifications", checked); });
	connect(this->googleAnalytics, &SettingsSwitch::clicked, [](bool checked) { PotatoConfig().set("use_ga", checked); });
	connect(this->statsMode->btnGroup, &QButtonGroup::idClicked, [](int id) { PotatoConfig().set<int>("stats_mode", id); });
	connect(this->csv, &SettingsSwitch::clicked, [](bool checked) { PotatoConfig().set<bool>("save_csv", checked); });
    connect(this->language->btnGroup, &QButtonGroup::idClicked, [this](int id) {
        PotatoConfig().set<int>("language", id);
        QEvent event(QEvent::LanguageChange);
        QApplication::sendEvent(this->window(), &event);
    });
	connect(this->gamePathButton, &QToolButton::clicked, [this]() { 
		QString dir = QFileDialog::getExistingDirectory(this, "Select Game Directory", "", QFileDialog::ShowDirsOnly);
		if (dir != "")
		{
			this->gamePathEdit->setText(dir);
            PotatoConfig().set("game_folder", dir.toStdString());
			this->checkPath();
		}
	});
}

void SettingsWidget::checkPath()
{
    auto path = PotatoConfig().get<std::string>("game_folder");
    folderStatus status = Game::checkPath(path);
    this->folderStatusGui->updateStatus(status);
    this->pc->setFolderStatus(status);
}

void SettingsWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        this->updateLabel->setText(GetString(Keys::SETTINGS_UPDATES));
        this->csvLabel->setText(GetString(Keys::SETTINGS_SAVE_CSV));
        this->gamePathLabel->setText(GetString(Keys::SETTINGS_GAME_DIRECTORY));
        this->statsModeLabel->setText(GetString(Keys::SETTINGS_STATS_MODE));
        this->gaLabel->setText(GetString(Keys::SETTINGS_GA));
        this->languageLabel->setText(GetString(Keys::SETTINGS_LANGUAGE));
        this->saveButton->setText(GetString(Keys::SETTINGS_SAVE));
        this->cancelButton->setText(GetString(Keys::SETTINGS_CANCEL));
    }
    else
    {
        QWidget::changeEvent(event);
    }
}
