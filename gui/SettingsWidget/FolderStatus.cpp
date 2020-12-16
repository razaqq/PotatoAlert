// Copyright 2020 <github.com/razaqq>

#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QString>
#include <QGridLayout>
#include <QEvent>
#include "FolderStatus.hpp"
#include "Game.hpp"
#include "StringTable.hpp"


using PotatoAlert::FolderStatus;

FolderStatus::FolderStatus(QWidget *parent) : QWidget(parent)
{
    this->init();
}

void FolderStatus::init()
{
    auto gridLayout = new QGridLayout;
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

    this->steamLabel->setBuddy(this->steamVersion);
    gridLayout->addWidget(this->steamLabel, 4, 0);
    gridLayout->addWidget(this->steamVersion, 4, 1);

    this->versionLabel->setBuddy(this->versionedReplays);
    gridLayout->addWidget(this->versionedLabel, 5, 0);
    gridLayout->addWidget(this->versionedReplays, 5, 1);

    this->setLayout(gridLayout);
}

void FolderStatus::updateStatus(folderStatus &status)
{
    this->statusText->setText(QString::fromStdString(status.statusText));
    if (status.found)
        this->statusText->setStyleSheet("QLabel { color : green; }");
    else
        this->statusText->setStyleSheet("QLabel { color : red; }");

    this->region->setText(QString::fromStdString(status.region));
    this->gameVersion->setText(QString::fromStdString(status.gameVersion));
    status.steamVersion ? this->steamVersion->setText("yes") : this->steamVersion->setText("no");
    status.versionedReplays ? this->versionedReplays->setText("yes") : this->versionedReplays->setText("no");

    this->replaysFolders->clear();
    if (status.replaysPath.size() == 1)
        this->replaysFolders->setText(QString::fromStdString(status.replaysPath[0]));
    else if (status.replaysPath.size() == 2)
        this->replaysFolders->setText(
                QString::fromStdString(status.replaysPath[0]) + "\n" + QString::fromStdString(status.replaysPath[1])
                );
}

void FolderStatus::changeEvent(QEvent* event)
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