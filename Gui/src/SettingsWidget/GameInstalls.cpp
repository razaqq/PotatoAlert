// Copyright 2024 <github.com/razaqq>

#include "Client/PotatoClient.hpp"
#include "Client/StringTable.hpp"

#include "Gui/ExpandingWidget.hpp"
#include "Gui/Events.hpp"
#include "Gui/IconButton.hpp"
#include "Gui/ScalingLabel.hpp"
#include "Gui/SettingsWidget/GameInstalls.hpp"

#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <filesystem>
#include <ranges>
#include <span>
#include <string>


namespace fs = std::filesystem;
using PotatoAlert::Gui::GameInstalls;
using PotatoAlert::Gui::LanguageChangeEvent;
using PotatoAlert::Gui::ScalingLabel;
using PotatoAlert::Client::GameDirectory;
using PotatoAlert::Client::StringTable::GetString;
using PotatoAlert::Client::StringTable::StringTableKey;

class GameInstall : public QWidget
{
public:
	explicit GameInstall(const GameDirectory& game, int lang)
	{
		qApp->installEventFilter(this);

		QGridLayout* gridLayout = new QGridLayout();
		gridLayout->setContentsMargins(10, 0, 10, 0);
		gridLayout->setVerticalSpacing(0);

		m_statusLabel->setBuddy(m_statusText);
		gridLayout->addWidget(m_statusLabel, 0, 0);
		gridLayout->addWidget(m_statusText, 0, 1);

		m_replaysLabel->setBuddy(m_replaysFolders);
		gridLayout->addWidget(m_replaysLabel, 1, 0);
		gridLayout->addWidget(m_replaysFolders, 1, 1);

		m_regionLabel->setBuddy(m_region);
		gridLayout->addWidget(m_regionLabel, 2, 0);
		gridLayout->addWidget(m_region, 2, 1);

		m_versionLabel->setBuddy(m_gameVersion);
		gridLayout->addWidget(m_versionLabel, 3, 0);
		gridLayout->addWidget(m_gameVersion, 3, 1);

		m_versionLabel->setBuddy(m_versionedReplays);
		gridLayout->addWidget(m_versionedLabel, 5, 0);
		gridLayout->addWidget(m_versionedReplays, 5, 1);

		setLayout(gridLayout);

		m_replaysFolders->clear();
		m_versionedReplays->clear();
		m_region->clear();
		m_gameVersion->clear();

		m_statusText->setText(game.Status.c_str());
		m_statusText->setStyleSheet(game.Info.has_value() ? "QLabel { color: green; }" : "QLabel { color: red; }");

		if (game.Info)
		{
			const GameInfo& info = *game.Info;

			m_region->setText(info.Region.c_str());
			m_gameVersion->setText(info.GameVersion.ToString().c_str());
			info.VersionedReplays ? m_versionedReplays->setText(GetString(lang, StringTableKey::YES))
								  : m_versionedReplays->setText(GetString(lang, StringTableKey::NO));

			// TODO: remove this once gcc has this
#if __cpp_lib_ranges_join_with
			const std::string replaysPaths =
				info.ReplaysPaths | std::views::transform([](const fs::path& p)
				{
					fs::path path = p;
					path.make_preferred();
					return path.native();
				}) | std::views::join_with('\n') | std::ranges::to<std::string>();
#else

			std::string replaysPaths;
			std::ranges::copy(
					info.ReplaysPaths | std::views::transform([](const fs::path& p)
					{
						fs::path path = p;
						path.make_preferred();
						return path.native();
					}) | std::views::join_with('\n'),
					std::back_inserter(replaysPaths));
#endif

			m_replaysFolders->setText(QString::fromStdString(replaysPaths));
		}

		SetLanguage(lang);
	}

	void SetLanguage(int lang) const
	{
		m_statusLabel->setText(GetString(lang, StringTableKey::SETTINGS_REPLAYSFOLDER_STATUS));
		m_replaysLabel->setText(GetString(lang, StringTableKey::SETTINGS_REPLAYSFOLDER_FOLDERS));
		m_regionLabel->setText(GetString(lang, StringTableKey::SETTINGS_REPLAYSFOLDER_REGION));
		m_versionLabel->setText(GetString(lang, StringTableKey::SETTINGS_REPLAYSFOLDER_GAMEVERSION));
		m_steamLabel->setText(GetString(lang, StringTableKey::SETTINGS_REPLAYSFOLDER_STEAM));
		m_versionedLabel->setText(GetString(lang, StringTableKey::SETTINGS_REPLAYSFOLDER_VERSIONED));
	}

protected:
	bool eventFilter(QObject* watched, QEvent* event) override
	{
		if (event->type() == LanguageChangeEvent::RegisteredType())
		{
			const int lang = dynamic_cast<LanguageChangeEvent*>(event)->GetLanguage();
			SetLanguage(lang);
		}
		return QWidget::eventFilter(watched, event);
	}

private:
	ScalingLabel* m_statusLabel = new ScalingLabel();
	ScalingLabel* m_replaysLabel = new ScalingLabel();
	ScalingLabel* m_regionLabel = new ScalingLabel();
	ScalingLabel* m_versionLabel = new ScalingLabel();
	ScalingLabel* m_steamLabel = new ScalingLabel();
	ScalingLabel* m_versionedLabel = new ScalingLabel();

	ScalingLabel* m_region = new ScalingLabel();
	ScalingLabel* m_gameVersion = new ScalingLabel();
	ScalingLabel* m_found = new ScalingLabel();
	ScalingLabel* m_replaysFolders = new ScalingLabel();
	ScalingLabel* m_versionedReplays = new ScalingLabel();
	ScalingLabel* m_statusText = new ScalingLabel();
};


GameInstalls::GameInstalls(QWidget* parent) : QWidget(parent)
{
	qApp->installEventFilter(this);

	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(15, 0, 15, 0);
	layout->setSpacing(5);
	setLayout(layout);
}

void GameInstalls::SetInstalls(std::span<const GameDirectory> infos)
{
	while (QLayoutItem const* item = layout()->takeAt(0))
	{
		delete item->widget();
		delete item;
	}

	const QFont labelFont(QApplication::font().family(), 10, QFont::Medium);

	for (const GameDirectory& game : infos)
	{
		QHBoxLayout* titleLayout = new QHBoxLayout();
		titleLayout->setContentsMargins(0, 4, 10, 4);
		titleLayout->setSpacing(0);

		ScalingLabel* titleLabel = new ScalingLabel(game.Path.string().c_str(), labelFont);
		titleLabel->setStyleSheet(game.Info.has_value() ? "QLabel { color: green; }" : "QLabel { color: red; }");
		titleLayout->addWidget(titleLabel, 0, Qt::AlignLeft);
		titleLayout->addStretch();
		IconButton* deleteButton = new IconButton(":/Close.svg", ":/CloseHover.svg", QSize(20, 20), false);
		connect(deleteButton, &IconButton::clicked, [this, &game](bool _)
		{
			emit RemoveGameInstall(game);
		});
		titleLayout->addWidget(deleteButton, 0, Qt::AlignRight);

		ExpandingWidget* ex = new ExpandingWidget(titleLayout);
		ex->setObjectName("GameInstall");
		GameInstall* install = new GameInstall(game, m_currentLang);

		QHBoxLayout* exLayout = new QHBoxLayout();
		exLayout->setContentsMargins(0, 3, 0, 3);
		exLayout->addWidget(install);
		exLayout->addStretch();

		ex->SetContentLayout(exLayout);
		layout()->addWidget(ex);
	}
}

bool GameInstalls::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == LanguageChangeEvent::RegisteredType())
	{
		m_currentLang = dynamic_cast<LanguageChangeEvent*>(event)->GetLanguage();
	}
	return QWidget::eventFilter(watched, event);
}
