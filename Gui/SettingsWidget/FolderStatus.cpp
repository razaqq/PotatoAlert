// Copyright 2020 <github.com/razaqq>

#include "FolderStatus.hpp"

#include "Client/Game.hpp"
#include "Core/StringTable.hpp"

#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include <sstream>


using PotatoAlert::Gui::FolderStatus;

FolderStatus::FolderStatus(QWidget* parent) : QWidget(parent)
{
	this->Init();
}

void FolderStatus::Init()
{
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

void FolderStatus::Update(const Client::Game::FolderStatus& status) const
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

void FolderStatus::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		this->m_statusLabel->setText(GetString(StringTable::Keys::SETTINGS_REPLAYSFOLDER_STATUS));
		this->m_replaysLabel->setText(GetString(StringTable::Keys::SETTINGS_REPLAYSFOLDER_FOLDERS));
		this->m_regionLabel->setText(GetString(StringTable::Keys::SETTINGS_REPLAYSFOLDER_REGION));
		this->m_versionLabel->setText(GetString(StringTable::Keys::SETTINGS_REPLAYSFOLDER_GAMEVERSION));
		this->m_steamLabel->setText(GetString(StringTable::Keys::SETTINGS_REPLAYSFOLDER_STEAM));
		this->m_versionedLabel->setText(GetString(StringTable::Keys::SETTINGS_REPLAYSFOLDER_VERSIONED));
	}
	else
	{
		QWidget::changeEvent(event);
	}
}
