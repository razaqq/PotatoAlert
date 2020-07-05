// Copyright 2020 <github.com/razaqq>

#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QString>
#include <QGridLayout>
#include "FolderStatus.h"
#include "Game.h"

using PotatoAlert::FolderStatus;

FolderStatus::FolderStatus(QWidget *parent) : QWidget(parent)
{
    this->init();
}

void FolderStatus::init()
{
    auto gridLayout = new QGridLayout;
    gridLayout->setContentsMargins(10, 0, 10, 0);

    auto statusLabel = new QLabel("Status:");
    statusLabel->setBuddy(this->statusText);
    gridLayout->addWidget(statusLabel, 0, 0);
    gridLayout->addWidget(this->statusText, 0, 1);

    auto replaysLabel = new QLabel("Replays Folders:");
    replaysLabel->setBuddy(this->replaysFolders);
    gridLayout->addWidget(replaysLabel, 1, 0);
    gridLayout->addWidget(this->replaysFolders, 1, 1);

    auto regionLabel = new QLabel("Region:");
    regionLabel->setBuddy(this->region);
    gridLayout->addWidget(regionLabel, 2, 0);
    gridLayout->addWidget(this->region, 2, 1);

    auto versionLabel = new QLabel("Game Version:");
    versionLabel->setBuddy(this->gameVersion);
    gridLayout->addWidget(versionLabel, 3, 0);
    gridLayout->addWidget(this->gameVersion, 3, 1);

    auto steamLabel = new QLabel("Steam:");
    steamLabel->setBuddy(this->steamVersion);
    gridLayout->addWidget(steamLabel, 4, 0);
    gridLayout->addWidget(this->steamVersion, 4, 1);

    auto versionedLabel = new QLabel("Versioned Replays:");
    versionLabel->setBuddy(this->versionedReplays);
    gridLayout->addWidget(versionedLabel, 5, 0);
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
