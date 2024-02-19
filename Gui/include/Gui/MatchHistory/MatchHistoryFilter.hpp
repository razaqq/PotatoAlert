// Copyright 2024 <github.com/razaqq>
#pragma once

#include "Client/DatabaseManager.hpp"
#include "Client/StringTable.hpp"

#include "Gui/FramelessDialog.hpp"

#include <QAbstractListModel>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include <set>
#include <span>
#include <string>


namespace PotatoAlert::Gui {

struct FilterState
{
	size_t Count;
	bool IsChecked;
};

using Filter = std::map<QString, FilterState>;

class FilterModel : public QAbstractListModel
{
public:
	explicit FilterModel(Filter& filter) : m_filter(filter)
	{
	}

	int rowCount(const QModelIndex& parent) const override
	{
		return static_cast<int>(m_filter.size());
	}

	QVariant data(const QModelIndex& index, int role) const override
	{
		if (!index.isValid())
			return QVariant();

		if (role == Qt::DisplayRole)
			return std::next(m_filter.begin(), index.row())->first;

		if (role == Qt::CheckStateRole)
			return std::next(m_filter.begin(), index.row())->second.IsChecked ? Qt::Checked : Qt::Unchecked;

		return QVariant();
	}

	bool setData(const QModelIndex& index, const QVariant& value, int role) override
	{
		if (!index.isValid())
			return false;

		if (role == Qt::CheckStateRole)
		{
			const Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
			m_filter.at(std::next(m_filter.begin(), index.row())->first).IsChecked = state == Qt::Checked ? true : false;
			emit dataChanged(index, index, { role });
			return true;
		}

		return QAbstractListModel::setData(index, value, role);
	}

	Qt::ItemFlags flags(const QModelIndex& index) const override
	{
		if (!index.isValid())
			return Qt::NoItemFlags;

		return QAbstractListModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
	}

	void AllDataChanged()
	{
		emit dataChanged(QModelIndex(), QModelIndex());
	}

private:
	Filter& m_filter;
};

class FilterList : public QWidget
{
	Q_OBJECT

public:
	explicit FilterList(Client::StringTable::StringTableKey key, QWidget* parent = nullptr);

	void AddItem(std::string_view value, bool isChecked = true)
	{
		if (m_filter.contains(value.data()))
		{
			m_filter[value.data()].Count++;
		}
		else
		{
			m_filter.emplace(value.data(), FilterState{ 1, isChecked });
			m_model->AllDataChanged();
		}
	}

	void RemoveItem(std::string_view value)
	{
		if (m_filter.contains(value.data()))
		{
			if (--m_filter[value.data()].Count == 0)
			{
				m_filter.erase(value.data());
				m_model->AllDataChanged();
			}
		}
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
	Filter m_filter;
	FilterModel* m_model = new FilterModel(m_filter);
	Client::StringTable::StringTableKey m_groupKey;
	QGroupBox* m_groupBox = new QGroupBox();
	QPushButton* m_toggle = new QPushButton();

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
	void Add(const Client::Match& match) const;
	void Remove(const Client::Match& match) const;

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
