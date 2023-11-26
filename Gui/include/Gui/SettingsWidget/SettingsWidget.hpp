// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/StringTable.hpp"

#include "Gui/IconButton.hpp"
#include "Gui/SettingsWidget/FolderStatus.hpp"
#include "Gui/SettingsWidget/SettingsChoice.hpp"
#include "Gui/SettingsWidget/SettingsSwitch.hpp"

#include <QComboBox>
#include <QEvent>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QWidget>


namespace PotatoAlert::Gui {

class SettingsWidget : public QWidget
{
	Q_OBJECT

private:
	const Client::ServiceProvider& m_services;

	FolderStatus* m_folderStatusGui = new FolderStatus(this);

	bool m_forceRun = false;

public:
	explicit SettingsWidget(const Client::ServiceProvider& serviceProvider, QWidget* parent = nullptr);
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void Init();
	template<typename SettingType, Client::ConfigKey Key>
	void AddSetting(QGridLayout* layout, SettingType* form, Client::StringTable::StringTableKey stringKey, auto&& onChange);

signals:
	void Done();
	void TableLayoutChanged();
	void LanguageChanged(int lang);
	void Reset();
};

}  // namespace PotatoAlert::Gui
