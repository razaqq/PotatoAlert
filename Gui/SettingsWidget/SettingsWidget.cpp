// Copyright 2020 <github.com/razaqq>

#include "SettingsWidget.hpp"

#include "Config.hpp"
#include "FolderStatus.hpp"
#include "HorizontalLine.hpp"
#include "PotatoClient.hpp"
#include "SettingsChoice.hpp"
#include "SettingsSwitch.hpp"
#include "StringTable.hpp"

#include <QApplication>
#include <QEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QPixmap>
#include <QPushButton>
#include <QSize>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>


static const int LABEL_WIDTH = 280;
static const int ROW_HEIGHT = 20;

using PotatoAlert::SettingsWidget;
using PotatoAlert::SettingsChoice;

SettingsWidget::SettingsWidget(QWidget* parent) : QWidget(parent)
{
	this->Init();
	this->ConnectSignals();
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
	this->m_updateLabel->setFont(labelFont);
	this->m_updateLabel->setFixedWidth(LABEL_WIDTH);
	updateLayout->addWidget(this->m_updateLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	updateLayout->addWidget(this->m_updates, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(updateLayout);
	/* UPDATE NOTIFICATIONS */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* SELECTOR FOR GAME FOLDER */
	auto gamePathLayout = new QHBoxLayout();
	this->m_gamePathLabel->setFont(labelFont);
	this->m_gamePathLabel->setFixedWidth(LABEL_WIDTH);
	gamePathLayout->addWidget(this->m_gamePathLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	gamePathLayout->addStretch();

	this->m_gamePathEdit->setFixedSize(278, ROW_HEIGHT);
	this->m_gamePathEdit->setReadOnly(true);
	this->m_gamePathEdit->setFocusPolicy(Qt::NoFocus);
	gamePathLayout->addWidget(this->m_gamePathEdit, 0, Qt::AlignVCenter | Qt::AlignRight);

	this->m_gamePathButton->setIcon(QIcon(QPixmap(":/folder.svg")));
	this->m_gamePathButton->setIconSize(QSize(ROW_HEIGHT, ROW_HEIGHT));
	this->m_gamePathButton->setCursor(Qt::PointingHandCursor);
	gamePathLayout->addWidget(this->m_gamePathButton, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(gamePathLayout);

	layout->addWidget(this->m_folderStatusGui);
	/* SELECTOR FOR GAME FOLDER */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* DISPLAYED STATS MODE */
	auto statsModeLayout = new QHBoxLayout();
	this->m_statsModeLabel->setFont(labelFont);
	this->m_statsModeLabel->setFixedWidth(LABEL_WIDTH);
	statsModeLayout->addWidget(this->m_statsModeLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	this->m_statsMode = new SettingsChoice(this, std::vector<QString>{"current mode", "pvp", "ranked", "clan"});  // TODO: localize
	this->m_statsMode->setEnabled(false);  // TODO: implement this on the backend
	statsModeLayout->addWidget(this->m_statsMode, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(statsModeLayout);
	/* DISPLAYED STATS MODE */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* LANGUAGE */
	auto languageLayout = new QHBoxLayout;
	this->m_languageLabel->setFont(labelFont);
	this->m_languageLabel->setFixedWidth(LABEL_WIDTH);
	languageLayout->addWidget(this->m_languageLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	std::vector<QString> langs;
	for (auto& lang : StringTable::Languages)
	{
		langs.push_back(QString::fromUtf8(lang.data()));
	}
	this->m_language = new SettingsChoice(this, langs);
	languageLayout->addWidget(this->m_language, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(languageLayout);
	/* LANGUAGE */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* CSV OUTPUT */
	auto csvLayout = new QHBoxLayout();
	this->m_matchHistoryLabel->setFixedWidth(LABEL_WIDTH);
	this->m_matchHistoryLabel->setFont(labelFont);
	this->m_matchHistoryLabel->setFixedWidth(LABEL_WIDTH);
	csvLayout->addWidget(this->m_matchHistoryLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	csvLayout->addWidget(this->m_matchHistory, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(csvLayout);
	/* CSV OUTPUT */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* MANUAL REPLAYS FOLDER */
	this->m_toggleReplaysFolderOverride = [this](bool override)
	{
		this->m_replaysFolderLabel->setEnabled(override);
		this->m_replaysFolderDesc->setEnabled(override);
		this->m_replaysFolderButton->setEnabled(override);
		this->m_replaysFolderEdit->setEnabled(override);

		this->m_gamePathButton->setDisabled(override);
		this->m_gamePathEdit->setDisabled(override);
		this->m_gamePathLabel->setDisabled(override);
		this->m_folderStatusGui->setDisabled(override);
	};

	auto replaysFolderVLayout = new QVBoxLayout();

	auto replaysFolderFirstRowLayout = new QHBoxLayout();
	replaysFolderFirstRowLayout->setContentsMargins(0, 0, 0, 0);
	this->m_replaysFolderLabel->setFont(labelFont);
	this->m_replaysFolderLabel->setFixedWidth(LABEL_WIDTH);
	replaysFolderFirstRowLayout->addWidget(this->m_replaysFolderLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	replaysFolderFirstRowLayout->addWidget(this->m_overrideReplaysFolder, 0, Qt::AlignVCenter | Qt::AlignRight);

	auto replaysFolderSecondRowLayout = new QHBoxLayout();
	replaysFolderSecondRowLayout->setContentsMargins(0, 0, 0, 0);

	replaysFolderSecondRowLayout->addWidget(this->m_replaysFolderDesc, 0, Qt::AlignVCenter | Qt::AlignLeft);
	replaysFolderSecondRowLayout->addStretch();

	this->m_replaysFolderEdit->setFixedSize(278, ROW_HEIGHT);
	this->m_replaysFolderEdit->setReadOnly(true);
	this->m_replaysFolderEdit->setFocusPolicy(Qt::NoFocus);
	replaysFolderSecondRowLayout->addWidget(this->m_replaysFolderEdit, 0, Qt::AlignVCenter | Qt::AlignRight);

	this->m_replaysFolderButton = new QToolButton(this);
	this->m_replaysFolderButton->setIcon(QIcon(QPixmap(":/folder.svg")));
	this->m_replaysFolderButton->setIconSize(QSize(ROW_HEIGHT, ROW_HEIGHT));
	this->m_replaysFolderButton->setCursor(Qt::PointingHandCursor);
	replaysFolderSecondRowLayout->addWidget(this->m_replaysFolderButton, 0, Qt::AlignVCenter | Qt::AlignRight);

	replaysFolderVLayout->addLayout(replaysFolderFirstRowLayout);
	replaysFolderVLayout->addLayout(replaysFolderSecondRowLayout);

	layout->addLayout(replaysFolderVLayout);

	layout->addStretch();

	/* SAVE & CANCEL BUTTON */
	auto confirmLayout = new QHBoxLayout();
	this->m_saveButton = new QPushButton();
	this->m_saveButton->setFixedWidth(100);
	this->m_saveButton->setObjectName("settingsButton");

	this->m_cancelButton = new QPushButton();
	this->m_cancelButton->setFixedWidth(100);
	this->m_cancelButton->setObjectName("settingsButton");

	confirmLayout->addStretch();
	confirmLayout->addWidget(this->m_saveButton);
	confirmLayout->addWidget(this->m_cancelButton);
	confirmLayout->addStretch();
	layout->addLayout(confirmLayout);

	centralWidget->setLayout(layout);
	this->setLayout(horLayout);

	this->Load();
}

void SettingsWidget::Load()
{
	this->m_updates->setChecked(PotatoConfig().Get<bool>("update_notifications"));
	// this->googleAnalytics->setChecked(PotatoConfig().get<bool>("use_ga"));
	this->m_gamePathEdit->setText(QString::fromStdString(
			PotatoConfig().Get<std::string>("game_folder")));
	this->m_statsMode->m_btnGroup->button(PotatoConfig().Get<int>("stats_mode"))->setChecked(true);
	this->m_language->m_btnGroup->button(PotatoConfig().Get<int>("language"))->setChecked(true);
	this->m_matchHistory->setChecked(PotatoConfig().Get<bool>("match_history"));

	this->m_replaysFolderEdit->setText(QString::fromStdString(
			PotatoConfig().Get<std::string>("replays_folder")));
	bool manualReplays = PotatoConfig().Get<bool>("override_replays_folder");
	this->m_overrideReplaysFolder->setChecked(manualReplays);
	this->m_toggleReplaysFolderOverride(manualReplays);
}

void SettingsWidget::ConnectSignals()
{
	connect(this->m_saveButton, &QPushButton::clicked, [this]()
	{
		PotatoConfig().Save();
		this->CheckPath();
		emit this->Done();
	});
	connect(this->m_cancelButton, &QPushButton::clicked, [this]()
	{
		PotatoConfig().Load();
		this->Load();
		this->CheckPath();
		QEvent event(QEvent::LanguageChange);
		QApplication::sendEvent(this->window(), &event);
		emit this->Done();
	});
	connect(this->m_updates, &SettingsSwitch::clicked, [](bool checked) { PotatoConfig().Set<bool>("update_notifications", checked); });
	// connect(this->googleAnalytics, &SettingsSwitch::clicked, [](bool checked) { PotatoConfig().set("use_ga", checked); });
	connect(this->m_statsMode->m_btnGroup, &QButtonGroup::idClicked, [](int id) { PotatoConfig().Set<int>("stats_mode", id); });
	connect(this->m_matchHistory, &SettingsSwitch::clicked, [](bool checked) { PotatoConfig().Set<bool>("match_history", checked); });
	connect(this->m_language->m_btnGroup, &QButtonGroup::idClicked, [this](int id)
	{
		PotatoConfig().Set<int>("language", id);
		QEvent event(QEvent::LanguageChange);
		QApplication::sendEvent(this->window(), &event);
	});
	connect(this->m_gamePathButton, &QToolButton::clicked, [this]()
	{
		QString dir = QFileDialog::getExistingDirectory(this, "Select Game Directory", "", QFileDialog::ShowDirsOnly);
		if (dir != "")
		{
			this->m_gamePathEdit->setText(dir);
			PotatoConfig().Set("game_folder", dir.toStdString());
			this->CheckPath();
		}
	});
	connect(this->m_replaysFolderButton, &QToolButton::clicked, [this]()
	{
		QString dir = QFileDialog::getExistingDirectory(this, "Select Replays Folder", "", QFileDialog::ShowDirsOnly);
		if (dir != "")
		{
			this->m_replaysFolderEdit->setText(dir);
			PotatoConfig().Set("replays_folder", dir.toStdString());
		}
	});
	connect(this->m_overrideReplaysFolder, &SettingsSwitch::clicked, [this](bool checked)
	{
		PotatoConfig().Set<bool>("override_replays_folder", checked);
		this->m_toggleReplaysFolderOverride(checked);
	});
}

void SettingsWidget::CheckPath()
{
	this->m_folderStatusGui->Update(PotatoClient::Instance().CheckPath());
}

void SettingsWidget::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		this->m_updateLabel->setText(GetString(StringTable::Keys::SETTINGS_UPDATES));
		this->m_matchHistoryLabel->setText(GetString(StringTable::Keys::SETTINGS_SAVE_MATCHHISTORY));
		this->m_gamePathLabel->setText(GetString(StringTable::Keys::SETTINGS_GAME_DIRECTORY));
		this->m_replaysFolderLabel->setText(GetString(StringTable::Keys::SETTINGS_MANUAL_REPLAYS));
		this->m_replaysFolderDesc->setText(GetString(StringTable::Keys::SETTINGS_MANUAL_REPLAYS_DESC));
		this->m_statsModeLabel->setText(GetString(StringTable::Keys::SETTINGS_STATS_MODE));
		this->m_gaLabel->setText(GetString(StringTable::Keys::SETTINGS_GA));
		this->m_languageLabel->setText(GetString(StringTable::Keys::SETTINGS_LANGUAGE));
		this->m_saveButton->setText(GetString(StringTable::Keys::SETTINGS_SAVE));
		this->m_cancelButton->setText(GetString(StringTable::Keys::SETTINGS_CANCEL));
	}
	else
	{
		QWidget::changeEvent(event);
	}
}
