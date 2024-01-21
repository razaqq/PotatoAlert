// Copyright 2020 <github.com/razaqq>

#include "Client/Game.hpp"
#include "Client/StringTable.hpp"

#include "Core/String.hpp"

#include "Gui/Events.hpp"
#include "Gui/SettingsWidget/FolderStatus.hpp"

#include <QApplication>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QWidget>


using namespace PotatoAlert::Client::StringTable;
using namespace PotatoAlert::Core;
using PotatoAlert::Gui::FolderStatus;

FolderStatus::FolderStatus(QWidget* parent) : QWidget(parent)
{
	Init();
}

void FolderStatus::Init()
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
}

void FolderStatus::Update(const Client::Game::DirectoryStatus& status) const
{
	m_replaysFolders->clear();
	m_versionedReplays->clear();
	m_region->clear();
	m_gameVersion->clear();

	m_statusText->setText(status.statusText.c_str());
	if (status.found)
	{
		m_statusText->setStyleSheet("QLabel { color: green; }");
		m_region->setText(status.region.c_str());
		m_gameVersion->setText(status.gameVersion.ToString().c_str());
		status.versionedReplays ?
			m_versionedReplays->setText(GetString(m_currentLanguage, StringTableKey::YES))
			: m_versionedReplays->setText(GetString(m_currentLanguage, StringTableKey::NO));
		QString replaysFolders = "";
		for (size_t i = 0; i < status.replaysPath.size(); i++)
		{
			if (i > 0)
				replaysFolders += "\n";
			const std::filesystem::path path = std::filesystem::path(status.replaysPath[i]).make_preferred();
			replaysFolders += path.native();
		}
		m_replaysFolders->setText(replaysFolders);
	}
	else
	{
		m_statusText->setStyleSheet("QLabel { color: red; }");
	}
}

bool FolderStatus::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == LanguageChangeEvent::RegisteredType())
	{
		const int lang = dynamic_cast<LanguageChangeEvent*>(event)->GetLanguage();
		m_statusLabel->setText(GetString(lang, StringTableKey::SETTINGS_REPLAYSFOLDER_STATUS));
		m_replaysLabel->setText(GetString(lang, StringTableKey::SETTINGS_REPLAYSFOLDER_FOLDERS));
		m_regionLabel->setText(GetString(lang, StringTableKey::SETTINGS_REPLAYSFOLDER_REGION));
		m_versionLabel->setText(GetString(lang, StringTableKey::SETTINGS_REPLAYSFOLDER_GAMEVERSION));
		m_steamLabel->setText(GetString(lang, StringTableKey::SETTINGS_REPLAYSFOLDER_STEAM));
		m_versionedLabel->setText(GetString(lang, StringTableKey::SETTINGS_REPLAYSFOLDER_VERSIONED));
		m_currentLanguage = lang;
	}
	return QWidget::eventFilter(watched, event);
}
