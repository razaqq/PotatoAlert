// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/FontLoader.hpp"
#include "Client/PotatoClient.hpp"
#include "Client/ServiceProvider.hpp"
#include "Client/StringTable.hpp"

#include "Core/Directory.hpp"
#include "Core/TypeTraits.hpp"

#include "Gui/Events.hpp"
#include "Gui/Fonts.hpp"
#include "Gui/IconButton.hpp"
#include "Gui/ScalingLabel.hpp"
#include "Gui/SettingsWidget/GameInstalls.hpp"
#include "Gui/SettingsWidget/HorizontalLine.hpp"
#include "Gui/SettingsWidget/SettingsChoice.hpp"
#include "Gui/SettingsWidget/SettingsComboBox.hpp"
#include "Gui/SettingsWidget/SettingsSlider.hpp"
#include "Gui/SettingsWidget/SettingsSwitch.hpp"
#include "Gui/SettingsWidget/SettingsWidget.hpp"

#include <QApplication>
#include <QComboBox>
#include <QEvent>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSize>
#include <QSizePolicy>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>


static constexpr int ROW_HEIGHT = 20;

using namespace PotatoAlert::Client::StringTable;
using namespace PotatoAlert::Core;
using PotatoAlert::Client::ConfigKey;
using PotatoAlert::Gui::SettingsWidget;
using PotatoAlert::Gui::SettingsChoice;

class TabWidget : public QTabWidget
{
protected:
	void resizeEvent(QResizeEvent* event) override
	{
		tabBar()->setFont(QFont(QApplication::font().family(), 12, QFont::Medium));
		tabBar()->setFixedWidth(event->size().width());
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	}
};

class SettingsButton : public QPushButton
{
public:
	explicit SettingsButton()
	{
		setFixedWidth(100);
		setObjectName("settingsButton");
		setCursor(Qt::PointingHandCursor);
	}
};

SettingsWidget::SettingsWidget(const Client::ServiceProvider& serviceProvider, QWidget* parent)
	: QWidget(parent), m_services(serviceProvider)
{
	Init();
	emit Reset();
}

void SettingsWidget::Init()
{
	qApp->installEventFilter(this);

	const QFont labelFont(QApplication::font().family(), 11, QFont::Medium);

	QHBoxLayout* layout = new QHBoxLayout();

	QWidget* general = new QWidget();
	QGridLayout* generalLayout = new QGridLayout();
	general->setLayout(generalLayout);
	generalLayout->setVerticalSpacing(5);
	generalLayout->setHorizontalSpacing(50);

	QWidget* display = new QWidget();
	QGridLayout* displayLayout = new QGridLayout();
	display->setLayout(displayLayout);
	displayLayout->setVerticalSpacing(5);
	displayLayout->setHorizontalSpacing(30);

	Config& config = m_services.Get<Config>();
	Client::PotatoClient& potatoClient = m_services.Get<Client::PotatoClient>();

	connect(&potatoClient, &Client::PotatoClient::GameInfosChanged, m_gameInstalls, &GameInstalls::SetInstalls);
	connect(m_gameInstalls, &GameInstalls::RemoveGameInstall, [&config, &potatoClient](const Client::GameDirectory& game)
	{
		auto gameInstalls = config.Get<ConfigKey::GameDirectories>();
		if (const auto it = gameInstalls.find(game.Path); it != gameInstalls.end())
		{
			gameInstalls.erase(it);
			config.Set<ConfigKey::GameDirectories>(gameInstalls);
			potatoClient.UpdateGameInstalls();
		}
	});

	// GENERAL
	ScalingLabel* gamePathLabel = new ScalingLabel(labelFont);

	generalLayout->addWidget(gamePathLabel, 0, 0, Qt::AlignLeft);
	IconButton* gameAddButton = new IconButton(":/Plus.svg", ":/PlusHover.svg", QSize(ROW_HEIGHT, ROW_HEIGHT));
	connect(gameAddButton, &IconButton::clicked, [this, &config, &potatoClient]([[maybe_unused]] bool checked)
	{
		QString dir = QFileDialog::getExistingDirectory(this, "Select Game Directory", "", QFileDialog::ShowDirsOnly);
		if (dir != "")
		{
			auto gameInstalls = config.Get<ConfigKey::GameDirectories>();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
			gameInstalls.emplace(QDir(dir).filesystemAbsolutePath().make_preferred());
#else
			gameInstalls.emplace(ToFilesystemAbsolutePath(QDir(dir)).make_preferred());
#endif
			config.Set<ConfigKey::GameDirectories>(gameInstalls);
			potatoClient.UpdateGameInstalls();
		}
	});
	generalLayout->addWidget(gamePathLabel, 0, 0, Qt::AlignLeft);
	generalLayout->addWidget(gameAddButton, 0, 1, Qt::AlignRight);
	generalLayout->addWidget(m_gameInstalls, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);

	generalLayout->addWidget(new HorizontalLine(), 2, 0, 1, 2);

	AddSetting<SettingsSwitch, ConfigKey::MatchHistory>(generalLayout, new SettingsSwitch(), StringTableKey::SETTINGS_SAVE_MATCHHISTORY, [](SettingsSwitch*, bool) {});
	AddSetting<SettingsSwitch, ConfigKey::SaveMatchCsv>(generalLayout, new SettingsSwitch(), StringTableKey::SETTINGS_SAVE_CSV, [](SettingsSwitch*, bool) {});

	generalLayout->addWidget(new HorizontalLine(), 5, 0, 1, 2);
	
	AddSetting<SettingsSwitch, ConfigKey::MinimizeTray>(generalLayout, new SettingsSwitch(), StringTableKey::SETTINGS_MINIMIZETRAY, [](SettingsSwitch*, bool) {});
	AddSetting<SettingsSwitch, ConfigKey::UpdateNotifications>(generalLayout, new SettingsSwitch(), StringTableKey::SETTINGS_UPDATES, [](SettingsSwitch*, bool) {});
	AddSetting<SettingsSwitch, ConfigKey::AllowSendingUsageStats>(generalLayout, new SettingsSwitch(), StringTableKey::SETTINGS_ALLOW_SENDING_USAGE_STATS, [](SettingsSwitch*, bool) {});

	// DISPLAY
	AddSetting<SettingsComboBox, ConfigKey::Language>(displayLayout, new SettingsComboBox(Languages), StringTableKey::SETTINGS_LANGUAGE, [this](SettingsComboBox*, int id)
	{
		LanguageChangeEvent event(id);
		QApplication::sendEvent(window(), &event);
	});
	SettingsChoice* statsMode = new SettingsChoice(this, { "current mode", "randoms", "ranked", "coop" });  // TODO: localize
	AddSetting<SettingsChoice, ConfigKey::StatsMode>(displayLayout, statsMode, StringTableKey::SETTINGS_STATS_MODE, [this](SettingsChoice*, int)
	{
		m_forceRun = true;
	});
	SettingsChoice* teamWinrateMode = new SettingsChoice(this, { "weighted", "average", "median" });  // TODO: localize
	AddSetting<SettingsChoice, ConfigKey::TeamWinRateMode>(displayLayout, teamWinrateMode, StringTableKey::SETTINGS_TEAM_WIN_RATE_MODE, [this](SettingsChoice*, int)
	{
		m_forceRun = true;
	});
	SettingsChoice* teamDamageMode = new SettingsChoice(this, { "weighted", "average", "median" });  // TODO: localize
	AddSetting<SettingsChoice, ConfigKey::TeamDamageMode>(displayLayout, teamDamageMode, StringTableKey::SETTINGS_TEAM_DAMAGE_MODE, [this](SettingsChoice*, int)
	{
		m_forceRun = true;
	});
	AddSetting<SettingsSwitch, ConfigKey::ShowKarma>(displayLayout, new SettingsSwitch(), StringTableKey::SETTINGS_SHOW_KARMA, [](SettingsSwitch*, bool) {});
	AddSetting<SettingsSwitch, ConfigKey::FontShadow>(displayLayout, new SettingsSwitch(), StringTableKey::SETTINGS_FONT_SHADOW, [](SettingsSwitch*, bool) {});
	SettingsChoice* tableLayout = new SettingsChoice(this, { "horizontal", "vertical" });
	AddSetting<SettingsChoice, ConfigKey::TableLayout>(displayLayout, tableLayout, StringTableKey::SETTINGS_TABLE_LAYOUT, [this](SettingsChoice*, int)
	{
		emit TableLayoutChanged();
	});
	AddSetting<SettingsSwitch, ConfigKey::AnonymizePlayers>(displayLayout, new SettingsSwitch(), StringTableKey::SETTINGS_ANONYMIZE_PLAYER_NAMES_SCREENSHOT, [](SettingsSwitch*, bool) {});
	AddSetting<SettingsComboBox, ConfigKey::Font>(displayLayout, new SettingsComboBox(Client::Fonts), StringTableKey::SETTINGS_FONT, [](SettingsComboBox* form, int)
	{
		QFont font = QApplication::font();
		font.setFamily(form->currentText());
		QApplication::setFont(font);
	});

	SettingsSlider* fontScaling = new SettingsSlider(50, 150);
	fontScaling->setFixedWidth(250);
	AddSetting<SettingsSlider, ConfigKey::FontScaling>(displayLayout, fontScaling, StringTableKey::SETTINGS_FONT_SCALING, [](SettingsSlider*, int value)
	{
		FontScalingChangeEvent event((float)value / 100.0f);
		for (QWidget* w : qApp->allWidgets())
		{
			QApplication::sendEvent(w, &event);
		}
	});

	TabWidget* tabWidget = new TabWidget();
	tabWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	tabWidget->addTab(general, "");
	tabWidget->addTab(display, "");

	QHBoxLayout* horLayout = new QHBoxLayout();
	horLayout->addLayout(layout);

	QHBoxLayout* confirmLayout = new QHBoxLayout();
	SettingsButton* saveButton = new SettingsButton();
	SettingsButton* cancelButton = new SettingsButton();

	confirmLayout->addStretch();
	confirmLayout->addWidget(saveButton);
	confirmLayout->addWidget(cancelButton);
	confirmLayout->addStretch();

	QWidget* centralWidget = new QWidget();
	centralWidget->setObjectName("settingsWidget");
	QVBoxLayout* centralLayout = new QVBoxLayout();
	centralLayout->setContentsMargins(0, 0, 0, 10);
	centralLayout->addWidget(tabWidget, 0, Qt::AlignHCenter);
	centralLayout->addLayout(confirmLayout);
	centralWidget->setLayout(centralLayout);

	layout->addStretch();
	layout->addWidget(centralWidget);
	layout->addStretch();

	connect(saveButton, &QPushButton::clicked, [this, &config, &potatoClient]()
	{
		if (m_forceRun)
		{
			m_forceRun = false;
			m_services.Get<Client::PotatoClient>().ForceRun();
		}

		config.Save();
		potatoClient.UpdateGameInstalls();
		emit Done();
	});
	connect(cancelButton, &QPushButton::clicked, [this, &config, &potatoClient]()
	{
		m_forceRun = false;
		config.Load();
		emit Reset();
		potatoClient.UpdateGameInstalls();
		QEvent event(QEvent::LanguageChange);
		QApplication::sendEvent(window(), &event);
		emit Done();
	});
	connect(this, &SettingsWidget::LanguageChanged, [saveButton, cancelButton, gamePathLabel, tabWidget](int lang)
	{
		saveButton->setText(GetString(lang, StringTableKey::SETTINGS_SAVE));
		cancelButton->setText(GetString(lang, StringTableKey::SETTINGS_CANCEL));
		gamePathLabel->setText(GetString(lang, StringTableKey::SETTINGS_GAME_DIRECTORIES));
		tabWidget->setTabText(0, GetString(lang, StringTableKey::SETTINGS_GENERAL));
		tabWidget->setTabText(1, GetString(lang, StringTableKey::SETTINGS_DISPLAY));
	});

	generalLayout->setRowStretch(generalLayout->rowCount(), 1);
	displayLayout->setRowStretch(displayLayout->rowCount(), 1);

	setLayout(horLayout);
}

template<typename SettingType, ConfigKey Key>
void SettingsWidget::AddSetting(QGridLayout* layout, SettingType* form, StringTableKey stringKey, auto&& onChange)
{
	ScalingLabel* label = new ScalingLabel(QFont(QApplication::font().family(), 11, QFont::Bold));
	label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	connect(this, &SettingsWidget::LanguageChanged, [label, stringKey](int lang)
	{
		label->setText(GetString(lang, stringKey));
	});

	if constexpr (std::is_same_v<SettingType, SettingsSwitch>)
	{
		connect(this, &SettingsWidget::Reset, [this, form]()
		{
			form->setChecked(m_services.Get<Config>().Get<Key>());
		});
		connect(form, &SettingsSwitch::clicked, [this, onChange, form](bool checked)
		{
			m_services.Get<Config>().Set<Key>(checked);
			onChange(form, checked);
		});
	}
	else if constexpr (std::is_same_v<SettingType, SettingsComboBox>)
	{
		using ConfigType = decltype(m_services.Get<Config>().Get<Key>());
		form->setFixedHeight(ROW_HEIGHT);
		connect(this, &SettingsWidget::Reset, [this, form]()
		{
			ConfigType k = m_services.Get<Config>().Get<Key>();
			if constexpr (std::is_same_v<ConfigType, int>)
			{
				form->setCurrentIndex(k);
			}
			else if constexpr (std::is_same_v<ConfigType, std::string>)
			{
				form->setCurrentText(QString::fromStdString(k));
			}
			else
			{
				static_assert(always_false<>, "Unsupported config type");
			}
		});
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
		connect(form, &SettingsComboBox::currentIndexChanged, [this, onChange, form](int id)
#else
		connect(form, qOverload<int>(&SettingsComboBox::currentIndexChanged), [this, &onChange, form](int id)
#endif
		{
			if constexpr (std::is_same_v<ConfigType, int>)
			{
				m_services.Get<Config>().Set<Key>(id);
			}
			else if constexpr (std::is_same_v<ConfigType, std::string>)
			{
				const std::string k = form->itemText(id).toStdString();
				m_services.Get<Config>().Set<Key>(k);
			}
			else
			{
				static_assert(always_false<>, "Unsupported config type");
			}
			onChange(form, id);
		});
	}
	else if constexpr (std::is_same_v<SettingType, SettingsChoice>)
	{
		connect(this, &SettingsWidget::Reset, [this, form]()
		{
			form->SetCurrentIndex(static_cast<int>(m_services.Get<Config>().Get<Key>()));
		});
		connect(form, &SettingsChoice::CurrentIndexChanged, [this, onChange, form](int index)
		{
			m_services.Get<Config>().Set<Key>(static_cast<decltype(m_services.Get<Config>().Get<Key>())>(index));
			onChange(form, index);
		});
	}
	else if constexpr (std::is_same_v<SettingType, SettingsSlider>)
	{
		connect(this, &SettingsWidget::Reset, [this, form]()
		{
			form->SetValue(m_services.Get<Config>().Get<Key>());
		});
		connect(form, &SettingsSlider::ValueChanged, [this, onChange, form](int value)
		{
			m_services.Get<Config>().Set<Key>(value);
			onChange(form, value);
		});
	}
	else
	{
		static_assert(always_false<>, "Unknown SettingType");
	}

	QHBoxLayout* hLayout = new QHBoxLayout();
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->addStretch();
	hLayout->addWidget(form, 0, Qt::AlignRight);

	const int row = layout->rowCount();
	layout->addWidget(label, row, 0, Qt::AlignLeft | Qt::AlignTop);
	layout->addLayout(hLayout, row, 1, Qt::AlignRight | Qt::AlignTop);
}

bool SettingsWidget::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == LanguageChangeEvent::RegisteredType())
	{
		const int lang = dynamic_cast<LanguageChangeEvent*>(event)->GetLanguage();
		emit LanguageChanged(lang);
	}
	else if (event->type() == QEvent::ApplicationFontChange)
	{
		UpdateWidgetFont(this);
		return false;
	}
	return QWidget::eventFilter(watched, event);
}
