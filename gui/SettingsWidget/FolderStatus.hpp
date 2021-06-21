// Copyright 2020 <github.com/razaqq>
#pragma once

#include <QLabel>
#include <QWidget>
#include <QEvent>
#include "Game.hpp"

namespace PotatoAlert {

class FolderStatusGui : public QWidget
{
public:
	explicit FolderStatusGui(QWidget* parent);
	void updateStatus(const Game::FolderStatus& status);
private:
	void init();
	void changeEvent(QEvent* event) override;

	QLabel* statusLabel = new QLabel;
	QLabel* replaysLabel = new QLabel;
	QLabel* regionLabel = new QLabel;
	QLabel* versionLabel = new QLabel;
	QLabel* steamLabel = new QLabel;
	QLabel* versionedLabel = new QLabel;

	QLabel* region = new QLabel;
	QLabel* gameVersion = new QLabel;
	QLabel* found = new QLabel;
	QLabel* replaysFolders  = new QLabel;
	QLabel* versionedReplays = new QLabel;
	QLabel* statusText = new QLabel;
};

}  // namespace PotatoAlert
