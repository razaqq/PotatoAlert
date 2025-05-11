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

using PotatoAlert::Client::Config;
using PotatoAlert::Client::ConfigManager;
using namespace PotatoAlert::Client::StringTable;
using namespace PotatoAlert::Core;
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

	Config& config = m_services.Get<ConfigManager>().GetConfig();
	Client::PotatoClient& potatoClient = m_services.Get<Client::PotatoClient>();

	connect(&potatoClient, &Client::PotatoClient::GameInfosChanged, m_gameInstalls, &GameInstalls::SetInstalls);
	connect(m_gameInstalls, &GameInstalls::RemoveGameInstall, [&config, &potatoClient](const Client::GameDirectory& game)
	{
		auto gameInstalls = config.GameDirectories;
		if (const auto it = gameInstalls.find(game.Path); it != gameInstalls.end())
		{
			gameInstalls.erase(it);
			config.GameDirectories = gameInstalls;
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
			auto gameInstalls = config.GameDirectories;
			gameInstalls.emplace(QDir(dir).filesystemAbsolutePath().make_preferred());
			config.GameDirectories = gameInstalls;
			potatoClient.UpdateGameInstalls();
		}
	});
	generalLayout->addWidget(gamePathLabel, 0, 0, Qt::AlignLeft);
	generalLayout->addWidget(gameAddButton, 0, 1, Qt::AlignRight);
	generalLayout->addWidget(m_gameInstalls, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);

	generalLayout->addWidget(new HorizontalLine(), 2, 0, 1, 2);

	AddSetting<SettingsSwitch>(&Config::MatchHistory, generalLayout, new SettingsSwitch(), StringTableKey::SETTINGS_SAVE_MATCHHISTORY, [](SettingsSwitch*, bool) {});
	AddSetting<SettingsSwitch>(&Config::SaveMatchCsv, generalLayout, new SettingsSwitch(), StringTableKey::SETTINGS_SAVE_CSV, [](SettingsSwitch*, bool) {});

	generalLayout->addWidget(new HorizontalLine(), 5, 0, 1, 2);
	
	AddSetting<SettingsSwitch>(&Config::MinimizeTray, generalLayout, new SettingsSwitch(), StringTableKey::SETTINGS_MINIMIZETRAY, [](SettingsSwitch*, bool) {});
	AddSetting<SettingsSwitch>(&Config::UpdateNotifications, generalLayout, new SettingsSwitch(), StringTableKey::SETTINGS_UPDATES, [](SettingsSwitch*, bool) {});
	AddSetting<SettingsSwitch>(&Config::AllowSendingUsageStats, generalLayout, new SettingsSwitch(), StringTableKey::SETTINGS_ALLOW_SENDING_USAGE_STATS, [](SettingsSwitch*, bool) {});

	// DISPLAY
	AddSetting<SettingsComboBox>(&Config::Language, displayLayout, new SettingsComboBox(Languages), StringTableKey::SETTINGS_LANGUAGE, [this](SettingsComboBox*, int id)
	{
		LanguageChangeEvent event(id);
		QApplication::sendEvent(window(), &event);
	});
	SettingsChoice* statsMode = new SettingsChoice(this, { "current mode", "randoms", "ranked", "coop" });  // TODO: localize
	AddSetting<SettingsChoice>(&Config::StatsMode, displayLayout, statsMode, StringTableKey::SETTINGS_STATS_MODE, [this](SettingsChoice*, int)
	{
		m_forceRun = true;
	});
	SettingsChoice* teamWinrateMode = new SettingsChoice(this, { "weighted", "average", "median" });  // TODO: localize
	AddSetting<SettingsChoice>(&Config::TeamWinRateMode, displayLayout, teamWinrateMode, StringTableKey::SETTINGS_TEAM_WIN_RATE_MODE, [this](SettingsChoice*, int)
	{
		m_forceRun = true;
	});
	SettingsChoice* teamDamageMode = new SettingsChoice(this, { "weighted", "average", "median" });  // TODO: localize
	AddSetting<SettingsChoice>(&Config::TeamDamageMode, displayLayout, teamDamageMode, StringTableKey::SETTINGS_TEAM_DAMAGE_MODE, [this](SettingsChoice*, int)
	{
		m_forceRun = true;
	});
	AddSetting<SettingsSwitch>(&Config::ShowKarma, displayLayout, new SettingsSwitch(), StringTableKey::SETTINGS_SHOW_KARMA, [](SettingsSwitch*, bool) {});
	AddSetting<SettingsSwitch>(&Config::FontShadow, displayLayout, new SettingsSwitch(), StringTableKey::SETTINGS_FONT_SHADOW, [](SettingsSwitch*, bool) {});
	SettingsChoice* tableLayout = new SettingsChoice(this, { "horizontal", "vertical" });
	AddSetting<SettingsChoice>(&Config::TableLayout, displayLayout, tableLayout, StringTableKey::SETTINGS_TABLE_LAYOUT, [this](SettingsChoice*, int)
	{
		emit TableLayoutChanged();
	});
	AddSetting<SettingsSwitch>(&Config::AnonymizePlayers, displayLayout, new SettingsSwitch(), StringTableKey::SETTINGS_ANONYMIZE_PLAYER_NAMES_SCREENSHOT, [](SettingsSwitch*, bool) {});
	AddSetting<SettingsComboBox>(&Config::Font, displayLayout, new SettingsComboBox(Client::Fonts), StringTableKey::SETTINGS_FONT, [](SettingsComboBox* form, int)
	{
		QFont font = QApplication::font();
		font.setFamily(form->currentText());
		QApplication::setFont(font);
	});

	SettingsSlider* fontScaling = new SettingsSlider(50, 150);
	fontScaling->setFixedWidth(250);
	AddSetting<SettingsSlider>(&Config::FontScaling, displayLayout, fontScaling, StringTableKey::SETTINGS_FONT_SCALING, [](SettingsSlider*, int value)
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

	connect(saveButton, &QPushButton::clicked, [this, &potatoClient]()
	{
		if (m_forceRun)
		{
			m_forceRun = false;
			m_services.Get<Client::PotatoClient>().ForceRun();
		}

		m_services.Get<ConfigManager>().Save();  // TODO: handle result
		potatoClient.UpdateGameInstalls();
		emit Done();
	});
	connect(cancelButton, &QPushButton::clicked, [this, &potatoClient]()
	{
		m_forceRun = false;
		m_services.Get<ConfigManager>().Load();  // TODO: handle result
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

template<typename SettingType, typename Target>
void SettingsWidget::AddSetting(Target target, QGridLayout* layout, SettingType* form, StringTableKey stringKey, auto&& onChange)
{
	ScalingLabel* label = new ScalingLabel(QFont(QApplication::font().family(), 11, QFont::Bold));
	label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	connect(this, &SettingsWidget::LanguageChanged, [label, stringKey](int lang)
	{
		label->setText(GetString(lang, stringKey));
	});

	Config& config = m_services.Get<ConfigManager>().GetConfig();

	using ConfigType = std::decay_t<decltype(config.*target)>;

	if constexpr (std::is_same_v<SettingType, SettingsSwitch>)
	{
		connect(this, &SettingsWidget::Reset, [form, &config, target]()
		{
			form->setChecked(config.*target);
		});
		connect(form, &SettingsSwitch::clicked, [onChange, form, &config, target](bool checked)
		{
			config.*target = checked;
			onChange(form, checked);
		});
	}
	else if constexpr (std::is_same_v<SettingType, SettingsComboBox>)
	{
		form->setFixedHeight(ROW_HEIGHT);
		connect(this, &SettingsWidget::Reset, [form, &config, target]()
		{
			ConfigType k = config.*target;
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

		connect(form, &SettingsComboBox::currentIndexChanged, [onChange, form, &config, target](int id)
		{
			if constexpr (std::is_same_v<ConfigType, int>)
			{
				config.*target = id;
			}
			else if constexpr (std::is_same_v<ConfigType, std::string>)
			{
				const std::string k = form->itemText(id).toStdString();
				config.*target = k;
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
		connect(this, &SettingsWidget::Reset, [form, &config, target]()
		{
			form->SetCurrentIndex(static_cast<int>(config.*target));
		});
		connect(form, &SettingsChoice::CurrentIndexChanged, [onChange, form, &config, target](int index)
		{
			config.*target = static_cast<ConfigType>(index);
			onChange(form, index);
		});
	}
	else if constexpr (std::is_same_v<SettingType, SettingsSlider>)
	{
		connect(this, &SettingsWidget::Reset, [form, &config, target]()
		{
			form->SetValue(config.*target);
		});
		connect(form, &SettingsSlider::ValueChanged, [onChange, form, &config, target](int value)
		{
			config.*target = value;
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
