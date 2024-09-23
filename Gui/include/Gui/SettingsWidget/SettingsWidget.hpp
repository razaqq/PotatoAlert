// Copyright 2020 <github.com/razaqq>
#pragma once

#include "Client/Config.hpp"
#include "Client/StringTable.hpp"

#include "Gui/SettingsWidget/GameInstalls.hpp"

#include <QEvent>
#include <QGridLayout>
#include <QWidget>


namespace PotatoAlert::Gui {

class SettingsWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SettingsWidget(const Client::ServiceProvider& serviceProvider, QWidget* parent = nullptr);
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void Init();
	template<typename SettingType, Client::ConfigKey Key>
	void AddSetting(QGridLayout* layout, SettingType* form, Client::StringTable::StringTableKey stringKey, auto&& onChange);

private:
	const Client::ServiceProvider& m_services;
	GameInstalls* m_gameInstalls = new GameInstalls();
	bool m_forceRun = false;

signals:
	void Done();
	void TableLayoutChanged();
	void LanguageChanged(int lang);
	void Reset();
};

}  // namespace PotatoAlert::Gui
