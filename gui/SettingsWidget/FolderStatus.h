// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QLabel>
#include <QWidget>
#include "Game.h"

namespace PotatoAlert {

class FolderStatus : public QWidget
{
public:
    explicit FolderStatus(QWidget* parent);
    void updateStatus(folderStatus& status);
private:
    void init();

    QLabel* region = new QLabel;
    QLabel* gameVersion = new QLabel;
    QLabel* found = new QLabel;
    QLabel* steamVersion = new QLabel;
    QLabel* replaysFolders  = new QLabel;
    QLabel* versionedReplays = new QLabel;
    QLabel* statusText = new QLabel;
};

}  // namespace PotatoAlert
