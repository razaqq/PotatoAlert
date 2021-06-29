// Copyright 2020 <github.com/razaqq>

#include "FolderStatus.hpp"
#include "Game.hpp"
#include "StringTable.hpp"
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QString>
#include <QGridLayout>
#include <QEvent>
#include <sstream>


using PotatoAlert::FolderStatusGui;
using PotatoAlert::Game::FolderStatus;

FolderStatusGui::FolderStatusGui(QWidget *parent) : QWidget(parent)
{
	this->Init();
}

void FolderStatusGui::Init()
{
	auto gridLayout = new QGridLayout();
	gridLayout->setContentsMargins(10, 0, 10, 0);

	this->statusLabel->setBuddy(this->statusText);
	gridLayout->addWidget(this->statusLabel, 0, 0);
	gridLayout->addWidget(this->statusText, 0, 1);

	this->replaysLabel->setBuddy(this->replaysFolders);
	gridLayout->addWidget(this->replaysLabel, 1, 0);
	gridLayout->addWidget(this->replaysFolders, 1, 1);

	this->regionLabel->setBuddy(this->region);
	gridLayout->addWidget(this->regionLabel, 2, 0);
	gridLayout->addWidget(this->region, 2, 1);

	this->versionLabel->setBuddy(this->gameVersion);
	gridLayout->addWidget(this->versionLabel, 3, 0);
	gridLayout->addWidget(this->gameVersion, 3, 1);

	this->versionLabel->setBuddy(this->versionedReplays);
	gridLayout->addWidget(this->versionedLabel, 5, 0);
	gridLayout->addWidget(this->versionedReplays, 5, 1);

	this->setLayout(gridLayout);
}

void FolderStatusGui::Update(const FolderStatus& status)
{
	this->replaysFolders->clear();
	this->versionedReplays->clear();
	this->region->clear();
	this->gameVersion->clear();

	this->statusText->setText(QString::fromStdString(status.statusText));
	if (status.found)
	{
		this->statusText->setStyleSheet("QLabel { color : green; }");
		this->region->setText(QString::fromStdString(status.region));
		this->gameVersion->setText(QString::fromStdString(status.gameVersion));
		status.versionedReplays ? this->versionedReplays->setText("yes") : this->versionedReplays->setText("no");

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
		this->replaysFolders->setText(QString::fromStdString(ss.str()));
	}
	else
	{
		this->statusText->setStyleSheet("QLabel { color : red; }");
	}
}

void FolderStatusGui::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		this->statusLabel->setText(GetString(PotatoAlert::StringKeys::SETTINGS_REPLAYSFOLDER_STATUS));
		this->replaysLabel->setText(GetString(PotatoAlert::StringKeys::SETTINGS_REPLAYSFOLDER_FOLDERS));
		this->regionLabel->setText(GetString(PotatoAlert::StringKeys::SETTINGS_REPLAYSFOLDER_REGION));
		this->versionLabel->setText(GetString(PotatoAlert::StringKeys::SETTINGS_REPLAYSFOLDER_GAMEVERSION));
		this->steamLabel->setText(GetString(PotatoAlert::StringKeys::SETTINGS_REPLAYSFOLDER_STEAM));
		this->versionedLabel->setText(GetString(PotatoAlert::StringKeys::SETTINGS_REPLAYSFOLDER_VERSIONED));
	}
	else
	{
		QWidget::changeEvent(event);
	}
}