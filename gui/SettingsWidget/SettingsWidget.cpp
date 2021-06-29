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
#include "SettingsWidget.hpp"
#include "Game.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "PotatoClient.hpp"
#include "SettingsSwitch.hpp"
#include "SettingsChoice.hpp"
#include "HorizontalLine.hpp"
#include "FolderStatus.hpp"
#include "StringTable.hpp"


static const int LABEL_WIDTH = 280;
static const int ROW_HEIGHT = 20;

using PotatoAlert::SettingsWidget;
using PotatoAlert::SettingsChoice;

SettingsWidget::SettingsWidget(QWidget* parent, PotatoClient* pc) : QWidget(parent)
{
	this->pc = pc;
	this->Init();
	this->ConnectSignals();
	this->CheckPath();
}

void SettingsWidget::Init()
{
	auto horLayout = new QHBoxLayout();
	horLayout->setContentsMargins(10, 10, 10, 10);
	horLayout->setSpacing(0);
	auto centralWidget = new QWidget(this);
	centralWidget->setObjectName("settingsWidget");
	horLayout->addStretch();
	horLayout->addWidget(centralWidget);
	horLayout->addStretch();

	QFont labelFont("Helvetica Neue", 13, QFont::Bold);
	labelFont.setStyleStrategy(QFont::PreferAntialias);

	auto layout = new QVBoxLayout();
	
	/* UPDATE NOTIFICATIONS */
	auto updateLayout = new QHBoxLayout();
	this->updateLabel->setFont(labelFont);
	this->updateLabel->setFixedWidth(LABEL_WIDTH);
	updateLayout->addWidget(this->updateLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	updateLayout->addWidget(this->updates, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(updateLayout);
	/* UPDATE NOTIFICATIONS */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* SELECTOR FOR GAME FOLDER */
	auto gamePathLayout = new QHBoxLayout();
	this->gamePathLabel->setFont(labelFont);
	this->gamePathLabel->setFixedWidth(LABEL_WIDTH);
	gamePathLayout->addWidget(this->gamePathLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	gamePathLayout->addStretch();

	this->gamePathEdit->setFixedSize(278, ROW_HEIGHT);
	this->gamePathEdit->setReadOnly(true);
	this->gamePathEdit->setFocusPolicy(Qt::NoFocus);
	gamePathLayout->addWidget(this->gamePathEdit, 0, Qt::AlignVCenter | Qt::AlignRight);

	this->gamePathButton->setIcon(QIcon(QPixmap(":/folder.svg")));
	this->gamePathButton->setIconSize(QSize(ROW_HEIGHT, ROW_HEIGHT));
	this->gamePathButton->setCursor(Qt::PointingHandCursor);
	gamePathLayout->addWidget(this->gamePathButton, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(gamePathLayout);

	layout->addWidget(this->folderStatusGui);
	/* SELECTOR FOR GAME FOLDER */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* DISPLAYED STATS MODE */
	auto statsModeLayout = new QHBoxLayout();
	this->statsModeLabel->setFont(labelFont);
	this->statsModeLabel->setFixedWidth(LABEL_WIDTH);
	statsModeLayout->addWidget(this->statsModeLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	this->statsMode = new SettingsChoice(this, std::vector<QString>{"current mode", "pvp", "ranked", "clan"});  // TODO: localize
	this->statsMode->setEnabled(false);  // TODO: implement this on the backend
	statsModeLayout->addWidget(this->statsMode, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(statsModeLayout);
	/* DISPLAYED STATS MODE */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* LANGUAGE */
	auto languageLayout = new QHBoxLayout;
	this->languageLabel->setFont(labelFont);
	this->languageLabel->setFixedWidth(LABEL_WIDTH);
	languageLayout->addWidget(this->languageLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	std::vector<QString> langs;
	for (auto& lang : Languages)
	{
		langs.push_back(QString::fromUtf8(lang.data()));
	}
	this->language = new SettingsChoice(this, langs);
	languageLayout->addWidget(this->language, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(languageLayout);
	/* LANGUAGE */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* CSV OUTPUT */
	auto csvLayout = new QHBoxLayout();
	this->csvLabel->setFixedWidth(LABEL_WIDTH);
	this->csvLabel->setFont(labelFont);
	this->csvLabel->setFixedWidth(LABEL_WIDTH);
	csvLayout->addWidget(this->csvLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	csvLayout->addWidget(this->csv, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(csvLayout);
	/* CSV OUTPUT */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* MANUAL REPLAYS FOLDER */
	this->toggleReplaysFolderOverride = [this](bool override)
	{
		this->replaysFolderLabel->setEnabled(override);
		this->replaysFolderDesc->setEnabled(override);
		this->replaysFolderButton->setEnabled(override);
		this->replaysFolderEdit->setEnabled(override);

		this->gamePathButton->setDisabled(override);
		this->gamePathEdit->setDisabled(override);
		this->gamePathLabel->setDisabled(override);
		this->folderStatusGui->setDisabled(override);
	};

	auto replaysFolderVLayout = new QVBoxLayout();

	auto replaysFolderFirstRowLayout = new QHBoxLayout();
	replaysFolderFirstRowLayout->setContentsMargins(0, 0, 0, 0);
	this->replaysFolderLabel->setFont(labelFont);
	this->replaysFolderLabel->setFixedWidth(LABEL_WIDTH);
	replaysFolderFirstRowLayout->addWidget(this->replaysFolderLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	replaysFolderFirstRowLayout->addWidget(this->overrideReplaysFolder, 0, Qt::AlignVCenter | Qt::AlignRight);

	auto replaysFolderSecondRowLayout = new QHBoxLayout();
	replaysFolderSecondRowLayout->setContentsMargins(0, 0, 0, 0);

	replaysFolderSecondRowLayout->addWidget(this->replaysFolderDesc, 0, Qt::AlignVCenter | Qt::AlignLeft);
	replaysFolderSecondRowLayout->addStretch();

	this->replaysFolderEdit->setFixedSize(278, ROW_HEIGHT);
	this->replaysFolderEdit->setReadOnly(true);
	this->replaysFolderEdit->setFocusPolicy(Qt::NoFocus);
	replaysFolderSecondRowLayout->addWidget(this->replaysFolderEdit, 0, Qt::AlignVCenter | Qt::AlignRight);

	this->replaysFolderButton = new QToolButton(this);
	this->replaysFolderButton->setIcon(QIcon(QPixmap(":/folder.svg")));
	this->replaysFolderButton->setIconSize(QSize(ROW_HEIGHT, ROW_HEIGHT));
	this->replaysFolderButton->setCursor(Qt::PointingHandCursor);
	replaysFolderSecondRowLayout->addWidget(this->replaysFolderButton, 0, Qt::AlignVCenter | Qt::AlignRight);

	replaysFolderVLayout->addLayout(replaysFolderFirstRowLayout);
	replaysFolderVLayout->addLayout(replaysFolderSecondRowLayout);

	layout->addLayout(replaysFolderVLayout);

	layout->addStretch();

	/* SAVE & CANCEL BUTTON */
	auto confirmLayout = new QHBoxLayout();
	this->saveButton = new QPushButton();
	this->saveButton->setFixedWidth(100);
	this->saveButton->setObjectName("settingsButton");

	this->cancelButton = new QPushButton();
	this->cancelButton->setFixedWidth(100);
	this->cancelButton->setObjectName("settingsButton");

	confirmLayout->addStretch();
	confirmLayout->addWidget(this->saveButton);
	confirmLayout->addWidget(this->cancelButton);
	confirmLayout->addStretch();
	layout->addLayout(confirmLayout);

	centralWidget->setLayout(layout);
	this->setLayout(horLayout);

	this->Load();
}

void SettingsWidget::Load()
{
	this->updates->setChecked(PotatoConfig().Get<bool>("update_notifications"));
	// this->googleAnalytics->setChecked(PotatoConfig().get<bool>("use_ga"));
	this->gamePathEdit->setText(QString::fromStdString(
			PotatoConfig().Get<std::string>("game_folder")));
	this->statsMode->btnGroup->button(PotatoConfig().Get<int>("stats_mode"))->setChecked(true);
	this->language->btnGroup->button(PotatoConfig().Get<int>("language"))->setChecked(true);
	this->csv->setChecked(PotatoConfig().Get<bool>("save_csv"));

	this->replaysFolderEdit->setText(QString::fromStdString(
			PotatoConfig().Get<std::string>("replays_folder")));
	bool manualReplays = PotatoConfig().Get<bool>("override_replays_folder");
	this->overrideReplaysFolder->setChecked(manualReplays);
	toggleReplaysFolderOverride.operator()(manualReplays);
}

void SettingsWidget::ConnectSignals()
{
	connect(this->saveButton, &QPushButton::clicked, [this]()
	{
		PotatoConfig().Save();
		this->CheckPath();
		emit this->done();
	});
	connect(this->cancelButton, &QPushButton::clicked, [this]()
	{
		PotatoConfig().Load();
		this->Load();
		this->CheckPath();
		QEvent event(QEvent::LanguageChange);
		QApplication::sendEvent(this->window(), &event);
		emit this->done();
	});
	connect(this->updates, &SettingsSwitch::clicked, [](bool checked) { PotatoConfig().Set<bool>("update_notifications", checked); });
	// connect(this->googleAnalytics, &SettingsSwitch::clicked, [](bool checked) { PotatoConfig().set("use_ga", checked); });
	connect(this->statsMode->btnGroup, &QButtonGroup::idClicked, [](int id) { PotatoConfig().Set<int>("stats_mode", id); });
	connect(this->csv, &SettingsSwitch::clicked, [](bool checked) { PotatoConfig().Set<bool>("save_csv", checked); });
	connect(this->language->btnGroup, &QButtonGroup::idClicked, [this](int id)
	{
		PotatoConfig().Set<int>("language", id);
		QEvent event(QEvent::LanguageChange);
		QApplication::sendEvent(this->window(), &event);
	});
	connect(this->gamePathButton, &QToolButton::clicked, [this]()
	{
		QString dir = QFileDialog::getExistingDirectory(this, "Select Game Directory", "", QFileDialog::ShowDirsOnly);
		if (dir != "")
		{
			this->gamePathEdit->setText(dir);
			PotatoConfig().Set("game_folder", dir.toStdString());
			this->CheckPath();
		}
	});
	connect(this->replaysFolderButton, &QToolButton::clicked, [this]()
	{
		QString dir = QFileDialog::getExistingDirectory(this, "Select Replays Folder", "", QFileDialog::ShowDirsOnly);
		if (dir != "")
		{
			this->replaysFolderEdit->setText(dir);
			PotatoConfig().Set("replays_folder", dir.toStdString());
		}
	});
	connect(this->overrideReplaysFolder, &SettingsSwitch::clicked, [this](bool checked)
	{
		PotatoConfig().Set<bool>("override_replays_folder", checked);
		this->toggleReplaysFolderOverride(checked);
	});
}

void SettingsWidget::CheckPath()
{
	FolderStatus status;
	if (Game::CheckPath(PotatoConfig().Get<std::string>("game_folder"), status))
	{
		this->pc->SetFolderStatus(status);
	}
	this->folderStatusGui->Update(status);
}

void SettingsWidget::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		this->updateLabel->setText(GetString(StringKeys::SETTINGS_UPDATES));
		this->csvLabel->setText(GetString(StringKeys::SETTINGS_SAVE_CSV));
		this->gamePathLabel->setText(GetString(StringKeys::SETTINGS_GAME_DIRECTORY));
		this->replaysFolderLabel->setText(GetString(StringKeys::SETTINGS_MANUAL_REPLAYS));
		this->replaysFolderDesc->setText(GetString(StringKeys::SETTINGS_MANUAL_REPLAYS_DESC));
		this->statsModeLabel->setText(GetString(StringKeys::SETTINGS_STATS_MODE));
		this->gaLabel->setText(GetString(StringKeys::SETTINGS_GA));
		this->languageLabel->setText(GetString(StringKeys::SETTINGS_LANGUAGE));
		this->saveButton->setText(GetString(StringKeys::SETTINGS_SAVE));
		this->cancelButton->setText(GetString(StringKeys::SETTINGS_CANCEL));
	}
	else
	{
		QWidget::changeEvent(event);
	}
}
