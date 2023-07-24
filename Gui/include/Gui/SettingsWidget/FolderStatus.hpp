// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/Game.hpp"

#include <QEvent>
#include <QLabel>
#include <QWidget>


namespace PotatoAlert::Gui {

class FolderStatus : public QWidget
{
public:
	explicit FolderStatus(QWidget* parent = nullptr);
	void Update(const Client::Game::DirectoryStatus& status) const;
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void Init();

private:
	QLabel* m_statusLabel = new QLabel();
	QLabel* m_replaysLabel = new QLabel();
	QLabel* m_regionLabel = new QLabel();
	QLabel* m_versionLabel = new QLabel();
	QLabel* m_steamLabel = new QLabel();
	QLabel* m_versionedLabel = new QLabel();

	QLabel* m_region = new QLabel();
	QLabel* m_gameVersion = new QLabel();
	QLabel* m_found = new QLabel();
	QLabel* m_replaysFolders = new QLabel();
	QLabel* m_versionedReplays = new QLabel();
	QLabel* m_statusText = new QLabel();
	int m_currentLanguage = 0;
};

}  // namespace PotatoAlert::Gui
