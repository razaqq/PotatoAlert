// Copyright 2020 <github.com/razaqq>

#include "Client/Game.hpp"
#include "Client/StringTable.hpp"

#include "Gui/SettingsWidget/FolderStatus.hpp"

#include "Gui/LanguageChangeEvent.hpp"

#include <QApplication>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include <sstream>


using namespace PotatoAlert::Client::StringTable;
using namespace PotatoAlert::Core;
using PotatoAlert::Gui::FolderStatus;

FolderStatus::FolderStatus(QWidget* parent) : QWidget(parent)
{
	this->Init();
}

void FolderStatus::Init()
{
	qApp->installEventFilter(this);

	auto gridLayout = new QGridLayout();
	gridLayout->setContentsMargins(10, 0, 10, 0);

	this->m_statusLabel->setBuddy(this->m_statusText);
	gridLayout->addWidget(this->m_statusLabel, 0, 0);
	gridLayout->addWidget(this->m_statusText, 0, 1);

	this->m_replaysLabel->setBuddy(this->m_replaysFolders);
	gridLayout->addWidget(this->m_replaysLabel, 1, 0);
	gridLayout->addWidget(this->m_replaysFolders, 1, 1);

	this->m_regionLabel->setBuddy(this->m_region);
	gridLayout->addWidget(this->m_regionLabel, 2, 0);
	gridLayout->addWidget(this->m_region, 2, 1);

	this->m_versionLabel->setBuddy(this->m_gameVersion);
	gridLayout->addWidget(this->m_versionLabel, 3, 0);
	gridLayout->addWidget(this->m_gameVersion, 3, 1);

	this->m_versionLabel->setBuddy(this->m_versionedReplays);
	gridLayout->addWidget(this->m_versionedLabel, 5, 0);
	gridLayout->addWidget(this->m_versionedReplays, 5, 1);

	this->setLayout(gridLayout);
}

void FolderStatus::Update(const Client::Game::DirectoryStatus& status) const
{
	this->m_replaysFolders->clear();
	this->m_versionedReplays->clear();
	this->m_region->clear();
	this->m_gameVersion->clear();

	this->m_statusText->setText(QString::fromStdString(status.statusText));
	if (status.found)
	{
		this->m_statusText->setStyleSheet("QLabel { color : green; }");
		this->m_region->setText(QString::fromStdString(status.region));
		this->m_gameVersion->setText(QString::fromStdString(status.gameVersion.ToString()));
		status.versionedReplays ? this->m_versionedReplays->setText("yes") : this->m_versionedReplays->setText("no");  // TODO: localize

		// create string from replays paths
		std::stringstream ss;
		auto beg = status.replaysPath.begin();
		auto end = status.replaysPath.end();
		if (beg != end)
		{
			ss << *beg;
			while (++beg != end)
				ss << "\n" << *beg;
		}
		this->m_replaysFolders->setText(QString::fromStdString(ss.str()));
	}
	else
	{
		this->m_statusText->setStyleSheet("QLabel { color : red; }");
	}
}

bool FolderStatus::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == LanguageChangeEvent::RegisteredType())
	{
		int lang = dynamic_cast<LanguageChangeEvent*>(event)->GetLanguage();
		m_statusLabel->setText(GetString(lang, StringTableKey::SETTINGS_REPLAYSFOLDER_STATUS));
		m_replaysLabel->setText(GetString(lang, StringTableKey::SETTINGS_REPLAYSFOLDER_FOLDERS));
		m_regionLabel->setText(GetString(lang, StringTableKey::SETTINGS_REPLAYSFOLDER_REGION));
		m_versionLabel->setText(GetString(lang, StringTableKey::SETTINGS_REPLAYSFOLDER_GAMEVERSION));
		m_steamLabel->setText(GetString(lang, StringTableKey::SETTINGS_REPLAYSFOLDER_STEAM));
		m_versionedLabel->setText(GetString(lang, StringTableKey::SETTINGS_REPLAYSFOLDER_VERSIONED));
	}
	return QWidget::eventFilter(watched, event);
}
