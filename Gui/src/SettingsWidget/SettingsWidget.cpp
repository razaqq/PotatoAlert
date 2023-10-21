// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/PotatoClient.hpp"
#include "Client/ServiceProvider.hpp"
#include "Client/StringTable.hpp"

#include "Core/Directory.hpp"

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
using PotatoAlert::Client::ConfigKey;
using PotatoAlert::Gui::SettingsWidget;
using PotatoAlert::Gui::SettingsChoice;

SettingsWidget::SettingsWidget(const Client::ServiceProvider& serviceProvider, QWidget* parent)
	: QWidget(parent), m_services(serviceProvider)
{
	Init();
	ConnectSignals();
}

void SettingsWidget::Init()
{
	qApp->installEventFilter(this);

	QHBoxLayout* horLayout = new QHBoxLayout();
	horLayout->setContentsMargins(10, 10, 10, 10);
	horLayout->setSpacing(0);
	QWidget* centralWidget = new QWidget(this);
	centralWidget->setObjectName("settingsWidget");
	horLayout->addStretch();
	horLayout->addWidget(centralWidget);
	horLayout->addStretch();

	// QFont labelFont("Helvetica Neue", 13, QFont::Bold);
	QFont labelFont("Noto Sans", 13, QFont::Bold);
	labelFont.setStyleStrategy(QFont::PreferAntialias);

	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(10, 10, 10, 10);
	
	/* UPDATE NOTIFICATIONS */
	QHBoxLayout* updateLayout = new QHBoxLayout();
	m_updateLabel->setFont(labelFont);
	m_updateLabel->setFixedWidth(LABEL_WIDTH);
	updateLayout->addWidget(m_updateLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	updateLayout->addWidget(m_updates, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(updateLayout);
	/* UPDATE NOTIFICATIONS */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* MINIMIZE TO TRAY */
	QHBoxLayout* minimizeTrayLayout = new QHBoxLayout();
	m_minimizeTrayLabel->setFont(labelFont);
	m_minimizeTrayLabel->setFixedWidth(LABEL_WIDTH);
	minimizeTrayLayout->addWidget(m_minimizeTrayLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	minimizeTrayLayout->addWidget(m_minimizeTray, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(minimizeTrayLayout);
	/* MINIMIZE TO TRAY */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* SELECTOR FOR GAME FOLDER */
	QHBoxLayout* gamePathLayout = new QHBoxLayout();
	m_gamePathLabel->setFont(labelFont);
	m_gamePathLabel->setFixedWidth(LABEL_WIDTH);
	gamePathLayout->addWidget(m_gamePathLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	gamePathLayout->addStretch();

	m_gamePathEdit->setFixedSize(278, ROW_HEIGHT);
	m_gamePathEdit->setReadOnly(true);
	m_gamePathEdit->setFocusPolicy(Qt::NoFocus);
	gamePathLayout->addWidget(m_gamePathEdit, 0, Qt::AlignVCenter | Qt::AlignRight);

	m_gamePathButton = new IconButton(":/Folder.svg", ":/FolderHover.svg", QSize(ROW_HEIGHT, ROW_HEIGHT));
	gamePathLayout->addWidget(m_gamePathButton, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(gamePathLayout);

	layout->addWidget(m_folderStatusGui);
	/* SELECTOR FOR GAME FOLDER */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* DISPLAYED STATS MODE */
	QHBoxLayout* statsModeLayout = new QHBoxLayout();
	m_statsModeLabel->setFont(labelFont);
	m_statsModeLabel->setFixedWidth(LABEL_WIDTH);
	statsModeLayout->addWidget(m_statsModeLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	m_statsMode = new SettingsChoice(this, { "current mode", "randoms", "ranked", "coop" });  // TODO: localize
	statsModeLayout->addWidget(m_statsMode, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(statsModeLayout);
	/* DISPLAYED STATS MODE */

	/* TEAM DAMAGE MODE */
	QHBoxLayout* teamDamageModeLayout = new QHBoxLayout();
	m_teamDamageModeLabel->setFont(labelFont);
	m_teamDamageModeLabel->setFixedWidth(LABEL_WIDTH);
	teamDamageModeLayout->addWidget(m_teamDamageModeLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	m_teamDamageMode = new SettingsChoice(this, { "weighted", "average", "median" });  // TODO: localize
	teamDamageModeLayout->addWidget(m_teamDamageMode, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(teamDamageModeLayout);
	/* TEAM DAMAGE MODE */

	/* TEAM WIN RATE MODE */
	QHBoxLayout* teamWinRateLayout = new QHBoxLayout();
	m_teamWinRateModeLabel->setFont(labelFont);
	m_teamWinRateModeLabel->setFixedWidth(LABEL_WIDTH);
	teamWinRateLayout->addWidget(m_teamWinRateModeLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	m_teamWinRateMode = new SettingsChoice(this, { "weighted", "average", "median" });  // TODO: localize
	teamWinRateLayout->addWidget(m_teamWinRateMode, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(teamWinRateLayout);
	/* TEAM WIN RATE MODE */

	/* SHOW KARMA */
	QHBoxLayout* showKarmaLayout = new QHBoxLayout();
	m_showKarmaLabel->setFont(labelFont);
	m_showKarmaLabel->setFixedWidth(LABEL_WIDTH);
	showKarmaLayout->addWidget(m_showKarmaLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	showKarmaLayout->addWidget(m_showKarma, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(showKarmaLayout);
	/* SHOW KARMA */

	/* FONT SHADOW */
	QHBoxLayout* fontShadowLayout = new QHBoxLayout();
	m_fontShadowLabel->setFont(labelFont);
	m_fontShadowLabel->setFixedWidth(LABEL_WIDTH);
	fontShadowLayout->addWidget(m_fontShadowLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	fontShadowLayout->addWidget(m_fontShadow, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(fontShadowLayout);
	/* FONT SHADOW */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* TABLE LAYOUT */
	QHBoxLayout* tableLayoutLayout = new QHBoxLayout();
	m_tableLayoutLabel->setFont(labelFont);
	m_tableLayoutLabel->setFixedWidth(LABEL_WIDTH);
	tableLayoutLayout->addWidget(m_tableLayoutLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	m_tableLayout = new SettingsChoice(this, { "horizontal", "vertical" });  // TODO: localize
	tableLayoutLayout->addWidget(m_tableLayout, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(tableLayoutLayout);
	/* TABLE LAYOUT */

	/* LANGUAGE */
	QHBoxLayout* languageLayout = new QHBoxLayout();
	m_languageLabel->setFont(labelFont);
	m_languageLabel->setFixedWidth(LABEL_WIDTH);
	languageLayout->addWidget(m_languageLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);

	for (const std::string_view lang : Languages)
	{
		m_language->addItem(lang.data());
	}
	m_language->setCursor(Qt::PointingHandCursor);
	languageLayout->addWidget(m_language, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(languageLayout);
	/* LANGUAGE */

	layout->addWidget(new HorizontalLine(centralWidget));

	/* MATCH HISTORY & CSV */
	QHBoxLayout* matchHistoryLayout = new QHBoxLayout();
	m_matchHistoryLabel->setFixedWidth(LABEL_WIDTH);
	m_matchHistoryLabel->setFont(labelFont);
	m_matchHistoryLabel->setFixedWidth(LABEL_WIDTH);
	matchHistoryLayout->addWidget(m_matchHistoryLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	matchHistoryLayout->addWidget(m_matchHistory, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(matchHistoryLayout);

	QHBoxLayout* csvLayout = new QHBoxLayout();
	m_saveMatchCsvLabel->setFixedWidth(LABEL_WIDTH);
	m_saveMatchCsvLabel->setFont(labelFont);
	m_saveMatchCsvLabel->setFixedWidth(LABEL_WIDTH);
	csvLayout->addWidget(m_saveMatchCsvLabel, 0, Qt::AlignVCenter | Qt::AlignLeft);
	csvLayout->addWidget(m_saveMatchCsv, 0, Qt::AlignVCenter | Qt::AlignRight);
	layout->addLayout(csvLayout);
	/* MATCH HISTORY & CSV */

	layout->addWidget(new HorizontalLine(centralWidget));
	
	layout->addStretch();

	/* SAVE & CANCEL BUTTON */
	QHBoxLayout* confirmLayout = new QHBoxLayout();
	m_saveButton = new QPushButton();
	m_saveButton->setFixedWidth(100);
	m_saveButton->setObjectName("settingsButton");
	m_saveButton->setCursor(Qt::PointingHandCursor);

	m_cancelButton = new QPushButton();
	m_cancelButton->setFixedWidth(100);
	m_cancelButton->setObjectName("settingsButton");
	m_cancelButton->setCursor(Qt::PointingHandCursor);

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
	const Config& config = m_services.Get<Config>();

	m_updates->setChecked(config.Get<ConfigKey::UpdateNotifications>());
	m_minimizeTray->setChecked(config.Get<ConfigKey::MinimizeTray>());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	m_gamePathEdit->setText(QDir(config.Get<ConfigKey::GameDirectory>().make_preferred()).absolutePath());
#else
	m_gamePathEdit->setText(FromFilesystemPath(config.Get<ConfigKey::GameDirectory>().make_preferred()).absolutePath());
#endif
	m_statsMode->GetButtonGroup()->button(static_cast<int>(config.Get<ConfigKey::StatsMode>()))->setChecked(true);
	m_teamDamageMode->GetButtonGroup()->button(static_cast<int>(config.Get<ConfigKey::TeamDamageMode>()))->setChecked(true);
	m_teamWinRateMode->GetButtonGroup()->button(static_cast<int>(config.Get<ConfigKey::TeamWinRateMode>()))->setChecked(true);
	m_tableLayout->GetButtonGroup()->button(static_cast<int>(config.Get<ConfigKey::TableLayout>()))->setChecked(true);
	m_language->setCurrentIndex(config.Get<ConfigKey::Language>());
	m_matchHistory->setChecked(config.Get<ConfigKey::MatchHistory>());
	m_saveMatchCsv->setChecked(config.Get<ConfigKey::SaveMatchCsv>());
	m_showKarma->setChecked(config.Get<ConfigKey::ShowKarma>());
	m_fontShadow->setChecked(config.Get<ConfigKey::FontShadow>());
}

void SettingsWidget::ConnectSignals()
{
	Config& config = m_services.Get<Config>();

	connect(m_saveButton, &QPushButton::clicked, [this, &config]()
	{
		if (m_forceRun)
		{
			m_forceRun = false;
			m_services.Get<Client::PotatoClient>().ForceRun();
		}

		config.Save();
		CheckPath();
		emit Done();
	});
	connect(m_cancelButton, &QPushButton::clicked, [this, &config]()
	{
		m_forceRun = false;
		config.Load();
		Load();
		CheckPath();
		QEvent event(QEvent::LanguageChange);
		QApplication::sendEvent(window(), &event);
		emit Done();
	});
	connect(m_updates, &SettingsSwitch::clicked, [&config](bool checked) { config.Set<ConfigKey::UpdateNotifications>(checked); });
	connect(m_minimizeTray, &SettingsSwitch::clicked, [&config](bool checked) { config.Set<ConfigKey::MinimizeTray>(checked); });
	connect(m_showKarma, &SettingsSwitch::clicked, [this, &config](bool checked)
	{
		config.Set<ConfigKey::ShowKarma>(checked);
	});
	connect(m_fontShadow, &SettingsSwitch::clicked, [this, &config](bool checked)
	{
		config.Set<ConfigKey::FontShadow>(checked);
	});
	connect(m_statsMode->GetButtonGroup(), &QButtonGroup::idClicked, [this, &config](int id)
	{
		m_forceRun = true;
		config.Set<ConfigKey::StatsMode>(static_cast<Client::StatsMode>(id));
	});
	connect(m_teamDamageMode->GetButtonGroup(), &QButtonGroup::idClicked, [this, &config](int id)
	{
		m_forceRun = true;
		config.Set<ConfigKey::TeamDamageMode>(static_cast<Client::TeamStatsMode>(id));
	});
	connect(m_teamWinRateMode->GetButtonGroup(), &QButtonGroup::idClicked, [this, &config](int id)
	{
		m_forceRun = true;
		config.Set<ConfigKey::TeamWinRateMode>(static_cast<Client::TeamStatsMode>(id));
	});
	connect(m_tableLayout->GetButtonGroup(), &QButtonGroup::idClicked, [this, &config](int id)
	{
		config.Set<ConfigKey::TableLayout>(static_cast<Client::TableLayout>(id));
		emit TableLayoutChanged();
	});
	connect(m_saveMatchCsv, &SettingsSwitch::clicked, [&config](bool checked) { config.Set<ConfigKey::SaveMatchCsv>(checked); });
	connect(m_matchHistory, &SettingsSwitch::clicked, [&config](bool checked) { config.Set<ConfigKey::MatchHistory>(checked); });
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
	connect(m_language, &QComboBox::currentIndexChanged, [this, &config](int id)
#else
	connect(m_language, qOverload<int>(&QComboBox::currentIndexChanged), [this, &config](int id)
#endif
	{
		config.Set<ConfigKey::Language>(id);
		LanguageChangeEvent event(id);
		QApplication::sendEvent(window(), &event);
	});
	connect(m_gamePathButton, &QToolButton::clicked, [this, &config]()
	{
		QString dir = QFileDialog::getExistingDirectory(this, "Select Game Directory", "", QFileDialog::ShowDirsOnly);
		if (dir != "")
		{
			m_gamePathEdit->setText(dir);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
			config.Set<ConfigKey::GameDirectory>(QDir(dir).filesystemAbsolutePath().make_preferred());
#else
			config.Set<ConfigKey::GameDirectory>(ToFilesystemAbsolutePath(QDir(dir)).make_preferred());
#endif
			CheckPath();
		}
	});
}

void SettingsWidget::CheckPath() const
{
	m_folderStatusGui->Update(m_services.Get<Client::PotatoClient>().CheckPath());
}

bool SettingsWidget::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == LanguageChangeEvent::RegisteredType())
	{
		const int lang = dynamic_cast<LanguageChangeEvent*>(event)->GetLanguage();
		m_updateLabel->setText(GetString(lang, StringTableKey::SETTINGS_UPDATES));
		m_minimizeTrayLabel->setText(GetString(lang, StringTableKey::SETTINGS_MINIMIZETRAY));
		m_saveMatchCsvLabel->setText(GetString(lang, StringTableKey::SETTINGS_SAVE_CSV));
		m_matchHistoryLabel->setText(GetString(lang, StringTableKey::SETTINGS_SAVE_MATCHHISTORY));
		m_gamePathLabel->setText(GetString(lang, StringTableKey::SETTINGS_GAME_DIRECTORY));
		m_statsModeLabel->setText(GetString(lang, StringTableKey::SETTINGS_STATS_MODE));
		m_teamDamageModeLabel->setText(GetString(lang, StringTableKey::SETTINGS_TEAM_DAMAGE_MODE));
		m_teamWinRateModeLabel->setText(GetString(lang, StringTableKey::SETTINGS_TEAM_WIN_RATE_MODE));
		m_tableLayoutLabel->setText(GetString(lang, StringTableKey::SETTINGS_TABLE_LAYOUT));
		m_languageLabel->setText(GetString(lang, StringTableKey::SETTINGS_LANGUAGE));
		m_saveButton->setText(GetString(lang, StringTableKey::SETTINGS_SAVE));
		m_cancelButton->setText(GetString(lang, StringTableKey::SETTINGS_CANCEL));
		m_showKarmaLabel->setText(GetString(lang, StringTableKey::SETTINGS_SHOW_KARMA));
		m_fontShadowLabel->setText(GetString(lang, StringTableKey::SETTINGS_FONT_SHADOW));
	}
	return QWidget::eventFilter(watched, event);
}
