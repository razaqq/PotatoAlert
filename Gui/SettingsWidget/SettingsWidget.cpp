// Copyright 2020 <github.com/razaqq>

#include "SettingsWidget.hpp"

#include "Client/PotatoClient.hpp"
#include "Core/Config.hpp"
#include "Core/StringTable.hpp"
#include "FolderStatus.hpp"
#include "HorizontalLine.hpp"
#include "SettingsChoice.hpp"
#include "SettingsSwitch.hpp"

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

using namespace PotatoAlert::Core;
using PotatoAlert::Gui::SettingsWidget;
using PotatoAlert::Gui::SettingsChoice;

SettingsWidget::SettingsWidget(QWidget* parent) : QWidget(parent)
{
	Init();
	ConnectSignals();
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
	m_updateLabel->setFont(labelFont);
	m_updateLabel->setFixedWidth(LABEL_WIDTH);
	updateLayout->addWidget(m_updateLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	updateLayout->addWidget(m_updates, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(updateLayout);
	/* UPDATE NOTIFICATIONS */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* MINIMIZE TO TRAY */
	auto minimizeTrayLayout = new QHBoxLayout();
	m_minimizeTrayLabel->setFont(labelFont);
	m_minimizeTrayLabel->setFixedWidth(LABEL_WIDTH);
	minimizeTrayLayout->addWidget(m_minimizeTrayLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	minimizeTrayLayout->addWidget(m_minimizeTray, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(minimizeTrayLayout);
	/* MINIMIZE TO TRAY */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* SELECTOR FOR GAME FOLDER */
	auto gamePathLayout = new QHBoxLayout();
	m_gamePathLabel->setFont(labelFont);
	m_gamePathLabel->setFixedWidth(LABEL_WIDTH);
	gamePathLayout->addWidget(m_gamePathLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	gamePathLayout->addStretch();

	m_gamePathEdit->setFixedSize(278, ROW_HEIGHT);
	m_gamePathEdit->setReadOnly(true);
	m_gamePathEdit->setFocusPolicy(Qt::NoFocus);
	gamePathLayout->addWidget(m_gamePathEdit, 0, Qt::AlignVCenter | Qt::AlignRight);

	m_gamePathButton->setIcon(QIcon(QPixmap(":/folder.svg")));
	m_gamePathButton->setIconSize(QSize(ROW_HEIGHT, ROW_HEIGHT));
	m_gamePathButton->setCursor(Qt::PointingHandCursor);
	gamePathLayout->addWidget(m_gamePathButton, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(gamePathLayout);

	layout->addWidget(m_folderStatusGui);
	/* SELECTOR FOR GAME FOLDER */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* DISPLAYED STATS MODE */
	auto statsModeLayout = new QHBoxLayout();
	m_statsModeLabel->setFont(labelFont);
	m_statsModeLabel->setFixedWidth(LABEL_WIDTH);
	statsModeLayout->addWidget(m_statsModeLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	m_statsMode = new SettingsChoice(this, std::vector<QString>{"current mode", "pvp", "ranked", "clan"});  // TODO: localize
	m_statsMode->setEnabled(false);  // TODO: implement this on the backend
	statsModeLayout->addWidget(m_statsMode, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(statsModeLayout);
	/* DISPLAYED STATS MODE */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* LANGUAGE */
	auto languageLayout = new QHBoxLayout;
	m_languageLabel->setFont(labelFont);
	m_languageLabel->setFixedWidth(LABEL_WIDTH);
	languageLayout->addWidget(m_languageLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	std::vector<QString> langs;
	for (auto& lang : StringTable::Languages)
	{
		langs.push_back(QString::fromUtf8(lang.data()));
	}
	m_language = new SettingsChoice(this, langs);
	languageLayout->addWidget(m_language, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(languageLayout);
	/* LANGUAGE */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* CSV OUTPUT */
	auto csvLayout = new QHBoxLayout();
	m_matchHistoryLabel->setFixedWidth(LABEL_WIDTH);
	m_matchHistoryLabel->setFont(labelFont);
	m_matchHistoryLabel->setFixedWidth(LABEL_WIDTH);
	csvLayout->addWidget(m_matchHistoryLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	csvLayout->addWidget(m_matchHistory, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(csvLayout);
	/* CSV OUTPUT */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* MANUAL REPLAYS FOLDER */
	m_toggleReplaysFolderOverride = [this](bool override)
	{
		m_replaysFolderLabel->setEnabled(override);
		m_replaysFolderDesc->setEnabled(override);
		m_replaysFolderButton->setEnabled(override);
		m_replaysFolderEdit->setEnabled(override);

		m_gamePathButton->setDisabled(override);
		m_gamePathEdit->setDisabled(override);
		m_gamePathLabel->setDisabled(override);
		m_folderStatusGui->setDisabled(override);
	};

	auto replaysFolderVLayout = new QVBoxLayout();

	auto replaysFolderFirstRowLayout = new QHBoxLayout();
	replaysFolderFirstRowLayout->setContentsMargins(0, 0, 0, 0);
	m_replaysFolderLabel->setFont(labelFont);
	m_replaysFolderLabel->setFixedWidth(LABEL_WIDTH);
	replaysFolderFirstRowLayout->addWidget(m_replaysFolderLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	replaysFolderFirstRowLayout->addWidget(m_overrideReplaysFolder, 0, Qt::AlignVCenter | Qt::AlignRight);

	auto replaysFolderSecondRowLayout = new QHBoxLayout();
	replaysFolderSecondRowLayout->setContentsMargins(0, 0, 0, 0);

	replaysFolderSecondRowLayout->addWidget(m_replaysFolderDesc, 0, Qt::AlignVCenter | Qt::AlignLeft);
	replaysFolderSecondRowLayout->addStretch();

	m_replaysFolderEdit->setFixedSize(278, ROW_HEIGHT);
	m_replaysFolderEdit->setReadOnly(true);
	m_replaysFolderEdit->setFocusPolicy(Qt::NoFocus);
	replaysFolderSecondRowLayout->addWidget(m_replaysFolderEdit, 0, Qt::AlignVCenter | Qt::AlignRight);

	m_replaysFolderButton = new QToolButton(this);
	m_replaysFolderButton->setIcon(QIcon(QPixmap(":/folder.svg")));
	m_replaysFolderButton->setIconSize(QSize(ROW_HEIGHT, ROW_HEIGHT));
	m_replaysFolderButton->setCursor(Qt::PointingHandCursor);
	replaysFolderSecondRowLayout->addWidget(m_replaysFolderButton, 0, Qt::AlignVCenter | Qt::AlignRight);

	replaysFolderVLayout->addLayout(replaysFolderFirstRowLayout);
	replaysFolderVLayout->addLayout(replaysFolderSecondRowLayout);

	layout->addLayout(replaysFolderVLayout);

	layout->addStretch();

	/* SAVE & CANCEL BUTTON */
	auto confirmLayout = new QHBoxLayout();
	m_saveButton = new QPushButton();
	m_saveButton->setFixedWidth(100);
	m_saveButton->setObjectName("settingsButton");

	m_cancelButton = new QPushButton();
	m_cancelButton->setFixedWidth(100);
	m_cancelButton->setObjectName("settingsButton");

	confirmLayout->addStretch();
	confirmLayout->addWidget(m_saveButton);
	confirmLayout->addWidget(m_cancelButton);
	confirmLayout->addStretch();
	layout->addLayout(confirmLayout);

	centralWidget->setLayout(layout);
	setLayout(horLayout);

	Load();
}

void SettingsWidget::Load()
{
	m_updates->setChecked(PotatoConfig().Get<bool>("update_notifications"));
	m_minimizeTray->setChecked(PotatoConfig().Get<bool>("minimize_tray"));
	m_gamePathEdit->setText(QString::fromStdString(
			PotatoConfig().Get<std::string>("game_folder")));
	m_statsMode->m_btnGroup->button(PotatoConfig().Get<int>("stats_mode"))->setChecked(true);
	m_language->m_btnGroup->button(PotatoConfig().Get<int>("language"))->setChecked(true);
	m_matchHistory->setChecked(PotatoConfig().Get<bool>("match_history"));

	m_replaysFolderEdit->setText(QString::fromStdString(
			PotatoConfig().Get<std::string>("replays_folder")));
	bool manualReplays = PotatoConfig().Get<bool>("override_replays_folder");
	m_overrideReplaysFolder->setChecked(manualReplays);
	m_toggleReplaysFolderOverride(manualReplays);
}

void SettingsWidget::ConnectSignals()
{
	connect(m_saveButton, &QPushButton::clicked, [this]()
	{
		PotatoConfig().Save();
		CheckPath();
		emit Done();
	});
	connect(m_cancelButton, &QPushButton::clicked, [this]()
	{
		PotatoConfig().Load();
		Load();
		CheckPath();
		QEvent event(QEvent::LanguageChange);
		QApplication::sendEvent(window(), &event);
		emit Done();
	});
	connect(m_updates, &SettingsSwitch::clicked, [](bool checked) { PotatoConfig().Set<bool>("update_notifications", checked); });
	connect(m_minimizeTray, &SettingsSwitch::clicked, [](bool checked) { PotatoConfig().Set<bool>("minimize_tray", checked); });
	connect(m_statsMode->m_btnGroup, &QButtonGroup::idClicked, [](int id) { PotatoConfig().Set<int>("stats_mode", id); });
	connect(m_matchHistory, &SettingsSwitch::clicked, [](bool checked) { PotatoConfig().Set<bool>("match_history", checked); });
	connect(m_language->m_btnGroup, &QButtonGroup::idClicked, [this](int id)
	{
		PotatoConfig().Set<int>("language", id);
		QEvent event(QEvent::LanguageChange);
		QApplication::sendEvent(window(), &event);
	});
	connect(m_gamePathButton, &QToolButton::clicked, [this]()
	{
		QString dir = QFileDialog::getExistingDirectory(this, "Select Game Directory", "", QFileDialog::ShowDirsOnly);
		if (dir != "")
		{
			m_gamePathEdit->setText(dir);
			PotatoConfig().Set("game_folder", dir.toStdString());
			CheckPath();
		}
	});
	connect(m_replaysFolderButton, &QToolButton::clicked, [this]()
	{
		QString dir = QFileDialog::getExistingDirectory(this, "Select Replays Folder", "", QFileDialog::ShowDirsOnly);
		if (dir != "")
		{
			m_replaysFolderEdit->setText(dir);
			PotatoConfig().Set("replays_folder", dir.toStdString());
		}
	});
	connect(m_overrideReplaysFolder, &SettingsSwitch::clicked, [this](bool checked)
	{
		PotatoConfig().Set<bool>("override_replays_folder", checked);
		m_toggleReplaysFolderOverride(checked);
	});
}

void SettingsWidget::CheckPath() const
{
	m_folderStatusGui->Update(Client::PotatoClient::Instance().CheckPath());
}

void SettingsWidget::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		m_updateLabel->setText(GetString(StringTable::Keys::SETTINGS_UPDATES));
		m_minimizeTrayLabel->setText(GetString(StringTable::Keys::SETTINGS_MINIMIZETRAY));
		m_matchHistoryLabel->setText(GetString(StringTable::Keys::SETTINGS_SAVE_MATCHHISTORY));
		m_gamePathLabel->setText(GetString(StringTable::Keys::SETTINGS_GAME_DIRECTORY));
		m_replaysFolderLabel->setText(GetString(StringTable::Keys::SETTINGS_MANUAL_REPLAYS));
		m_replaysFolderDesc->setText(GetString(StringTable::Keys::SETTINGS_MANUAL_REPLAYS_DESC));
		m_statsModeLabel->setText(GetString(StringTable::Keys::SETTINGS_STATS_MODE));
		m_gaLabel->setText(GetString(StringTable::Keys::SETTINGS_GA));
		m_languageLabel->setText(GetString(StringTable::Keys::SETTINGS_LANGUAGE));
		m_saveButton->setText(GetString(StringTable::Keys::SETTINGS_SAVE));
		m_cancelButton->setText(GetString(StringTable::Keys::SETTINGS_CANCEL));
	}
	else
	{
		QWidget::changeEvent(event);
	}
}
