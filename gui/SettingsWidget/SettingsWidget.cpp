// Copyright 2020 <github.com/razaqq>

#include <QIcon>
#include <QSize>
#include <QFrame>
#include <QPixmap>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QToolButton>
#include <QPushButton>
#include "SettingsWidget.h"
#include "Game.h"
#include "Config.h"
#include "Logger.h"
#include "PotatoClient.h"
#include "SettingsSwitch.h"
#include "SettingsChoice.h"
#include "HorizontalLine.h"
#include "FolderStatus.h"


const int LABELWIDTH = 200;
const int ROWHEIGHT = 20;
const int ROWWIDTH = 500;

using PotatoAlert::SettingsWidget;
using PotatoAlert::SettingsChoice;
using PotatoAlert::Game;

SettingsWidget::SettingsWidget(QWidget* parent, Config* c, Logger* l, PotatoClient* pc) : QWidget(parent)
{
	this->config = c;
	this->logger = l;
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
    auto updateLabel = new QLabel("Update Notifications");
	updateLabel->setFont(labelFont);
	updateLabel->setFixedWidth(LABELWIDTH);
	updateLayout->addWidget(updateLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	updateLayout->addWidget(this->updates, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(updateLayout);

	layout->addWidget(new HorizontalLine(centralWidget));

	/* CENTRAL API */
	auto centralApiLayout = new QHBoxLayout;
    auto centralApiLabel = new QLabel("Use Central API");
	centralApiLabel->setFixedWidth(LABELWIDTH);
	centralApiLabel->setFont(labelFont);
	centralApiLabel->setFixedWidth(LABELWIDTH);
	centralApiLayout->addWidget(centralApiLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	this->centralApi->setDisabled(true);
	centralApiLayout->addWidget(this->centralApi, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(centralApiLayout);

	layout->addWidget(new HorizontalLine(centralWidget));

	/* SELECTOR FOR GAME FOLDER */
    auto gamePathLayout = new QHBoxLayout;
    auto gamePathLabel = new QLabel("Game Directory");
	gamePathLabel->setFont(labelFont);
	gamePathLabel->setFixedWidth(LABELWIDTH);
	gamePathLayout->addWidget(gamePathLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	gamePathLayout->addStretch();

	this->gamePathEdit = new QLineEdit(centralWidget);
	this->gamePathEdit->setFixedSize(278, ROWHEIGHT);
	this->gamePathEdit->setReadOnly(true);
	this->gamePathEdit->setFocusPolicy(Qt::NoFocus);
	gamePathLayout->addWidget(this->gamePathEdit, 0, Qt::AlignVCenter | Qt::AlignRight);

	this->gamePathButton = new QToolButton();
	this->gamePathButton->setIcon(QIcon(QPixmap(":/folder.svg")));
	this->gamePathButton->setIconSize(QSize(ROWHEIGHT, ROWHEIGHT));
	this->gamePathButton->setCursor(Qt::PointingHandCursor);
	gamePathLayout->addWidget(this->gamePathButton, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(gamePathLayout);

	this->folderStatusGui = new FolderStatus(this);
    layout->addWidget(this->folderStatusGui);

	layout->addWidget(new HorizontalLine(centralWidget));

	/* DISPLAYED STATS MODE */
    auto statsModeLayout = new QHBoxLayout;
    auto statsModeLabel = new QLabel("Stats Mode");
	statsModeLabel->setFont(labelFont);
	statsModeLabel->setFixedWidth(LABELWIDTH);
	statsModeLayout->addWidget(statsModeLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	this->statsMode = new SettingsChoice(this, std::vector<QString>{"current mode", "pvp", "ranked", "clan"}); 
	statsModeLayout->addWidget(this->statsMode, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(statsModeLayout);

	layout->addWidget(new HorizontalLine(centralWidget));

	/* GOOGLE ANALYTICS */
    auto gaLayout = new QHBoxLayout;
    auto gaLabel = new QLabel("Allow Google Analytics");
	gaLabel->setFont(labelFont);
	gaLabel->setFixedWidth(LABELWIDTH);
	gaLayout->addWidget(gaLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	this->googleAnalytics->setDisabled(true);
	gaLayout->addWidget(this->googleAnalytics, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(gaLayout);

	layout->addStretch();

	/* SAVE & CANCEL BUTTON */
	// TODO: rework buttons
    auto confirmLayout = new QHBoxLayout;
	this->saveButton = new QPushButton("Save", this);
	this->cancelButton = new QPushButton("Cancel", this);
	confirmLayout->addWidget(this->saveButton);
	confirmLayout->addWidget(this->cancelButton);
	layout->addLayout(confirmLayout);

	centralWidget->setLayout(layout);
	this->setLayout(horLayout);

	this->load();
}

void SettingsWidget::load()
{
	this->updates->setChecked(this->config->get<bool>("update_notifications"));
	this->centralApi->setChecked(this->config->get<bool>("use_central_api"));
	this->googleAnalytics->setChecked(this->config->get<bool>("use_ga"));
	this->gamePathEdit->setText(QString::fromStdString(this->config->get<std::string>("game_folder")));
	this->statsMode->btnGroup->button(this->config->get<int>("stats_mode"))->setChecked(true);
}

void SettingsWidget::connectSignals()
{
	connect(this->saveButton, &QPushButton::clicked, [this]() { this->config->save(); });
	connect(this->cancelButton, &QPushButton::clicked, [this]() { this->config->load(); this->load(); this->checkPath(); });
	connect(this->updates, &SettingsSwitch::clicked, [this](bool checked) { this->config->set<bool>("update_notifications", checked); });
	connect(this->centralApi, &SettingsSwitch::clicked, [this](bool checked) { this->config->set("use_central_api", checked); });
	connect(this->googleAnalytics, &SettingsSwitch::clicked, [this](bool checked) { this->config->set("use_ga", checked); });
	connect(this->centralApi, &SettingsSwitch::clicked, [this](bool checked) { this->config->set<bool>("use_central_api", checked); });
	connect(this->statsMode->btnGroup, &QButtonGroup::idClicked, [this](int id) { this->config->set<int>("stats_mode", id); });
	connect(this->gamePathButton, &QToolButton::clicked, [this]() { 
		QString dir = QFileDialog::getExistingDirectory(this, "Select Game Directory", "", QFileDialog::ShowDirsOnly);
		if (dir != "")
		{
			this->gamePathEdit->setText(dir);
			this->config->set("game_folder", dir.toStdString());
			this->checkPath();
		}
	});
}

void SettingsWidget::checkPath()
{
    auto path = this->config->get<std::string>("game_folder");
    folderStatus status = Game::checkPath(path, this->logger);
    this->folderStatusGui->updateStatus(status);
    this->pc->setFolderStatus(status);
}
