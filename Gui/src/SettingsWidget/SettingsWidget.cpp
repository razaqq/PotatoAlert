// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/PotatoClient.hpp"
#include "Client/StringTable.hpp"

#include "Gui/SettingsWidget/FolderStatus.hpp"
#include "Gui/SettingsWidget/HorizontalLine.hpp"
#include "Gui/SettingsWidget/SettingsChoice.hpp"
#include "Gui/SettingsWidget/SettingsSwitch.hpp"
#include "Gui/SettingsWidget/SettingsWidget.hpp"

#include "Gui/LanguageChangeEvent.hpp"

#include <QApplication>
#include <QComboBox>
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


static constexpr int LABEL_WIDTH = 280;
static constexpr int ROW_HEIGHT = 20;

using namespace PotatoAlert::Client::StringTable;
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
	qApp->installEventFilter(this);

	auto horLayout = new QHBoxLayout();
	horLayout->setContentsMargins(10, 10, 10, 10);
	horLayout->setSpacing(0);
	auto centralWidget = new QWidget(this);
	centralWidget->setObjectName("settingsWidget");
	horLayout->addStretch();
	horLayout->addWidget(centralWidget);
	horLayout->addStretch();

	// QFont labelFont("Helvetica Neue", 13, QFont::Bold);
	QFont labelFont("Noto Sans", 13, QFont::Bold);
	labelFont.setStyleStrategy(QFont::PreferAntialias);

	auto layout = new QVBoxLayout();
	layout->setContentsMargins(10, 10, 10, 10);
	
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

	m_gamePathButton->setIcon(QIcon(QPixmap(":/Folder.svg")));
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

	m_statsMode = new SettingsChoice(this, { "current mode", "randoms", "ranked", "pve" });  // TODO: localize
	statsModeLayout->addWidget(m_statsMode, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(statsModeLayout);
	/* DISPLAYED STATS MODE */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* TEAM DAMAGE MODE */
	auto teamDamageModeLayout = new QHBoxLayout();
	m_teamDamageModeLabel->setFont(labelFont);
	m_teamDamageModeLabel->setFixedWidth(LABEL_WIDTH);
	teamDamageModeLayout->addWidget(m_teamDamageModeLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	m_teamDamageMode = new SettingsChoice(this, { "weighted", "average", "median" });  // TODO: localize
	teamDamageModeLayout->addWidget(m_teamDamageMode, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(teamDamageModeLayout);
	/* TEAM DAMAGE MODE */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* TEAM WIN RATE MODE */
	auto teamWinRateLayout = new QHBoxLayout();
	m_teamWinRateModeLabel->setFont(labelFont);
	m_teamWinRateModeLabel->setFixedWidth(LABEL_WIDTH);
	teamWinRateLayout->addWidget(m_teamWinRateModeLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	m_teamWinRateMode = new SettingsChoice(this, { "weighted", "average", "median" });  // TODO: localize
	teamWinRateLayout->addWidget(m_teamWinRateMode, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(teamWinRateLayout);
	/* TEAM WIN RATE MODE */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* LANGUAGE */
	auto languageLayout = new QHBoxLayout();
	m_languageLabel->setFont(labelFont);
	m_languageLabel->setFixedWidth(LABEL_WIDTH);
	languageLayout->addWidget(m_languageLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	for (const std::string_view lang : Languages)
	{
		m_language->addItem(lang.data());
	}
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

void SettingsWidget::Load() const
{
	m_updates->setChecked(PotatoConfig().Get<ConfigKey::UpdateNotifications>());
	m_minimizeTray->setChecked(PotatoConfig().Get<ConfigKey::MinimizeTray>());
	m_gamePathEdit->setText(QString::fromStdString(
			PotatoConfig().Get<ConfigKey::GameDirectory>()));
	m_statsMode->m_btnGroup->button(static_cast<int>(PotatoConfig().Get<ConfigKey::StatsMode>()))->setChecked(true);
	m_language->m_btnGroup->button(PotatoConfig().Get<ConfigKey::Language>())->setChecked(true);
	m_matchHistory->setChecked(PotatoConfig().Get<ConfigKey::MatchHistory>());

	// m_replaysFolderEdit->setText(QString::fromStdString(
	//		PotatoConfig().Get<ConfigKey::ReplaysDirectory>()));
	bool manualReplays = PotatoConfig().Get<ConfigKey::OverrideReplaysDirectory>();
	m_overrideReplaysFolder->setChecked(manualReplays);
	// m_toggleReplaysFolderOverride(manualReplays);
}

void SettingsWidget::ConnectSignals()
{
	connect(m_saveButton, &QPushButton::clicked, [this]()
	{
		if (m_forceRun)
		{
			m_forceRun = false;
			Client::PotatoClient::Instance().ForceRun();
		}

		PotatoConfig().Save();
		CheckPath();
		emit Done();
	});
	connect(m_cancelButton, &QPushButton::clicked, [this]()
	{
		m_forceRun = false;
		PotatoConfig().Load();
		Load();
		CheckPath();
		QEvent event(QEvent::LanguageChange);
		QApplication::sendEvent(window(), &event);
		emit Done();
	});
	connect(m_updates, &SettingsSwitch::clicked, [](bool checked) { PotatoConfig().Set<ConfigKey::UpdateNotifications>(checked); });
	connect(m_minimizeTray, &SettingsSwitch::clicked, [](bool checked) { PotatoConfig().Set<ConfigKey::MinimizeTray>(checked); });
	connect(m_statsMode->m_btnGroup, &QButtonGroup::idClicked, [this](int id)
	{
		m_forceRun = true;
		PotatoConfig().Set<ConfigKey::StatsMode>(static_cast<StatsMode>(id));
	});
	connect(m_matchHistory, &SettingsSwitch::clicked, [](bool checked) { PotatoConfig().Set<ConfigKey::MatchHistory>(checked); });
	connect(m_language->m_btnGroup, &QButtonGroup::idClicked, [this](int id)
	{
		PotatoConfig().Set<ConfigKey::Language>(id);
		QEvent event(QEvent::LanguageChange);
		QApplication::sendEvent(window(), &event);
	});
	connect(m_gamePathButton, &QToolButton::clicked, [this]()
	{
		QString dir = QFileDialog::getExistingDirectory(this, "Select Game Directory", "", QFileDialog::ShowDirsOnly);
		if (dir != "")
		{
			m_gamePathEdit->setText(dir);
			PotatoConfig().Set<ConfigKey::GameDirectory>(dir.toStdString());
			CheckPath();
		}
	});
}

void SettingsWidget::CheckPath() const
{
	m_folderStatusGui->Update(Client::PotatoClient::Instance().CheckPath());
}

bool SettingsWidget::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == LanguageChangeEvent::RegisteredType())
	{
		int lang = dynamic_cast<LanguageChangeEvent*>(event)->GetLanguage();
		m_updateLabel->setText(GetString(lang, StringTableKey::SETTINGS_UPDATES));
		m_minimizeTrayLabel->setText(GetString(lang, StringTableKey::SETTINGS_MINIMIZETRAY));
		m_matchHistoryLabel->setText(GetString(lang, StringTableKey::SETTINGS_SAVE_MATCHHISTORY));
		m_gamePathLabel->setText(GetString(lang, StringTableKey::SETTINGS_GAME_DIRECTORY));
		m_statsModeLabel->setText(GetString(lang, StringTableKey::SETTINGS_STATS_MODE));
		m_teamDamageModeLabel->setText(GetString(lang, StringTableKey::SETTINGS_TEAM_DAMAGE_MODE));
		m_teamWinRateModeLabel->setText(GetString(lang, StringTableKey::SETTINGS_TEAM_WIN_RATE_MODE));
		m_languageLabel->setText(GetString(lang, StringTableKey::SETTINGS_LANGUAGE));
		m_saveButton->setText(GetString(lang, StringTableKey::SETTINGS_SAVE));
		m_cancelButton->setText(GetString(lang, StringTableKey::SETTINGS_CANCEL));
	}
	return QWidget::eventFilter(watched, event);
}
