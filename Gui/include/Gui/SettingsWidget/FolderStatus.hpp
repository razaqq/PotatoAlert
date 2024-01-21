// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/Game.hpp"

#include "Gui/ScalingLabel.hpp"

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
	int m_currentLanguage = 0;
};

}  // namespace PotatoAlert::Gui
