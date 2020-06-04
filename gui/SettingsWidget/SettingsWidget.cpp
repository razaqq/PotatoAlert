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
#include "Config.h"
#include "SettingsSwitch.h"
#include "SettingsChoice.h"
#include "HorizontalLine.h"


const int LABELWIDTH = 200;
const int ROWHEIGHT = 20;
const int ROWWIDTH = 500;

using PotatoAlert::SettingsWidget;
using PotatoAlert::SettingsChoice;

SettingsWidget::SettingsWidget(QWidget* parent, Config* c) : QWidget(parent)
{
	this->config = c;
	this->init();
	this->connectSignals();
}

void SettingsWidget::init()
{
	// this->setStyleSheet("border: 1px solid red");

	QHBoxLayout* horLayout = new QHBoxLayout;
	horLayout->setContentsMargins(10, 10, 10, 10);
	horLayout->setSpacing(0);
	QWidget* centralWidget = new QWidget(this);
	centralWidget->setObjectName("settingsWidget");
	horLayout->addStretch();
	horLayout->addWidget(centralWidget);
	horLayout->addStretch();

	QFont labelFont("Helvetica Neue", 13, QFont::Bold);
	labelFont.setStyleStrategy(QFont::PreferAntialias);

	QVBoxLayout* layout = new QVBoxLayout;
	
	/* UPDATE NOTIFICATIONS */
	QHBoxLayout* updateLayout = new QHBoxLayout;
	QLabel* updateLabel = new QLabel("Update Notifications");
	updateLabel->setFont(labelFont);
	updateLabel->setFixedWidth(LABELWIDTH);
	updateLayout->addWidget(updateLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	updateLayout->addWidget(this->updates, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(updateLayout);

	layout->addWidget(new HorizontalLine(centralWidget));

	/* CENTRAL API */
	QHBoxLayout* centralApiLayout = new QHBoxLayout;
	QLabel* centralApiLabel = new QLabel("Use Central API");
	centralApiLabel->setFixedWidth(LABELWIDTH);
	centralApiLabel->setFont(labelFont);
	centralApiLabel->setFixedWidth(LABELWIDTH);
	centralApiLayout->addWidget(centralApiLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	this->centralApi->setDisabled(true);
	centralApiLayout->addWidget(this->centralApi, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(centralApiLayout);

	layout->addWidget(new HorizontalLine(centralWidget));

	/* SELECTOR FOR GAME FOLDER */
	QHBoxLayout* gamePathLayout = new QHBoxLayout;
	QLabel* gamePathLabel = new QLabel("Game Directory");
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

	layout->addWidget(new HorizontalLine(centralWidget));

	/* DISPLAYED STATS MODE */
	QHBoxLayout* statsModeLayout = new QHBoxLayout;
	QLabel* statsModeLabel = new QLabel("Stats Mode");
	statsModeLabel->setFont(labelFont);
	statsModeLabel->setFixedWidth(LABELWIDTH);
	statsModeLayout->addWidget(statsModeLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	this->statsMode = new SettingsChoice(this, std::vector<QString>{"current mode", "pvp", "ranked", "clan"}); 
	statsModeLayout->addWidget(this->statsMode, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(statsModeLayout);

	layout->addWidget(new HorizontalLine(centralWidget));

	/* GOOGLE ANALYTICS */
	QHBoxLayout* gaLayout = new QHBoxLayout;
	QLabel* gaLabel = new QLabel("Allow Google Analytics");
	gaLabel->setFont(labelFont);
	gaLabel->setFixedWidth(LABELWIDTH);
	gaLayout->addWidget(gaLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	this->googleAnalytics->setDisabled(true);
	gaLayout->addWidget(this->googleAnalytics, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(gaLayout);

	layout->addStretch();

	/* SAVE & CANCEL BUTTON */
	QHBoxLayout* confirmLayout = new QHBoxLayout;
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
	connect(this->cancelButton, &QPushButton::clicked, [this]() { this->config->load(); this->load(); });
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
		}
	});
}
