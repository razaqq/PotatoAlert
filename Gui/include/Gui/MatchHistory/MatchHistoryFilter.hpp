// Copyright 2024 <github.com/razaqq>
#pragma once

#include "Client/DatabaseManager.hpp"
#include "Client/StringTable.hpp"

#include "Gui/FramelessDialog.hpp"

#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include <set>
#include <span>
#include <string>


namespace PotatoAlert::Gui {

using Filter = std::map<QString, bool>;

class FilterList : public QWidget
{
	Q_OBJECT

public:
	explicit FilterList(Client::StringTable::StringTableKey key, QWidget* parent = nullptr);

	void AddItem(std::string_view value, bool isChecked = true)
	{
		m_filter.emplace(value.data(), isChecked);
	}

	void Clear()
	{
		m_filter.clear();
	}

	const Filter& GetFilter() const
	{
		return m_filter;
	}

	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	Client::StringTable::StringTableKey m_groupKey;
	QGroupBox* m_groupBox = new QGroupBox();
	QPushButton* m_toggle = new QPushButton();
	Filter m_filter;

signals:
	void FilterChanged();
};

class MatchHistoryFilter : public FramelessDialog
{
	Q_OBJECT

public:
	explicit MatchHistoryFilter(QWidget* align, QWidget* parent = nullptr);

	void AdjustPosition();
	void BuildFilter(std::span<const Client::Match> matches) const;

	[[nodiscard]] const Filter& ShipFilter() const { return m_shipList->GetFilter(); }
	[[nodiscard]] const Filter& MapFilter() const { return m_mapList->GetFilter(); }
	[[nodiscard]] const Filter& ModeFilter() const { return m_modeList->GetFilter(); }
	[[nodiscard]] const Filter& StatsModeFilter() const { return m_statsModeList->GetFilter(); }
	[[nodiscard]] const Filter& PlayerFilter() const { return m_playerList->GetFilter(); }
	[[nodiscard]] const Filter& RegionFilter() const { return m_regionList->GetFilter(); }

private:
	QWidget* m_align;

	FilterList* m_shipList = new FilterList(Client::StringTable::StringTableKey::COLUMN_SHIP, this);
	FilterList* m_mapList = new FilterList(Client::StringTable::StringTableKey::HISTORY_MAP, this);
	FilterList* m_modeList = new FilterList(Client::StringTable::StringTableKey::HISTORY_MODE, this);
	FilterList* m_statsModeList = new FilterList(Client::StringTable::StringTableKey::SETTINGS_STATS_MODE, this);
	FilterList* m_playerList = new FilterList(Client::StringTable::StringTableKey::COLUMN_PLAYER, this);
	FilterList* m_regionList = new FilterList(Client::StringTable::StringTableKey::HISTORY_REGION, this);

signals:
	void FilterChanged();
};

}  // namespace PotatoAlert::Gui
