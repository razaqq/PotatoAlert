// Copyright 2022 <github.com/razaqq>
#pragma once

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include <ctime>
#include <limits>


namespace PotatoAlert::Gui {

class MatchHistorySortFilter : public QSortFilterProxyModel
{
public:
	explicit MatchHistorySortFilter(MatchHistoryFilter* filter, QObject* parent = nullptr) : QSortFilterProxyModel(parent), m_filter(filter)
	{
		connect(filter, &MatchHistoryFilter::FilterChanged, [this]()
		{
			invalidateFilter();
		});
	}

	bool lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight) const override
	{
		if (sourceLeft.column() != 0)
		{
			// this never gets used
			return sourceModel()->data(sourceLeft, Qt::DisplayRole).toString() < sourceModel()->data(sourceRight, Qt::DisplayRole).toString();
		}
		else
		{
			return sourceModel()->data(sourceLeft, Qt::UserRole).value<std::time_t>() < sourceModel()->data(sourceRight, Qt::UserRole).value<std::time_t>();
		}
	}

	bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
	{
		const QModelIndex dateIndex = sourceModel()->index(sourceRow, 0, sourceParent);
		const QModelIndex shipIndex = sourceModel()->index(sourceRow, 1, sourceParent);
		const QModelIndex mapIndex = sourceModel()->index(sourceRow, 2, sourceParent);
		const QModelIndex modeIndex = sourceModel()->index(sourceRow, 3, sourceParent);
		const QModelIndex statsModeIndex = sourceModel()->index(sourceRow, 4, sourceParent);
		const QModelIndex playerIndex = sourceModel()->index(sourceRow, 5, sourceParent);
		const QModelIndex regionIndex = sourceModel()->index(sourceRow, 6, sourceParent);

		const std::time_t time = dateIndex.data(Qt::UserRole).value<std::time_t>();

		const QString ship = shipIndex.data(Qt::DisplayRole).toString();
		const QString map = mapIndex.data(Qt::DisplayRole).toString();
		const QString mode = modeIndex.data(Qt::DisplayRole).toString();
		const QString statsMode = statsModeIndex.data(Qt::DisplayRole).toString();
		const QString player = playerIndex.data(Qt::DisplayRole).toString();
		const QString region = regionIndex.data(Qt::DisplayRole).toString();

		auto isChecked = [this](const Filter& filter, const QString& key) -> bool
		{
			return filter.contains(key) && filter.at(key);
		};

		return time >= m_from && time <= m_to
			&& isChecked(m_filter->ShipFilter(), ship)
			&& isChecked(m_filter->MapFilter(), map)
			&& isChecked(m_filter->ModeFilter(), mode)
			&& isChecked(m_filter->StatsModeFilter(), statsMode)
			&& isChecked(m_filter->PlayerFilter(), player)
			&& isChecked(m_filter->RegionFilter(), region);
	}

	void SetFilterRange(std::time_t from = std::numeric_limits<std::time_t>::min(), std::time_t to = std::numeric_limits<std::time_t>::max())
	{
		assert(to >= from);
		m_from = from;
		m_to = to;
		invalidateFilter();
	}

	void ResetFilter()
	{
		m_from = std::numeric_limits<std::time_t>::min();
		m_to = std::numeric_limits<std::time_t>::max();
		invalidateFilter();
	}

private:
	std::time_t m_from = std::numeric_limits<std::time_t>::min();
	std::time_t m_to = std::numeric_limits<std::time_t>::max();
	const MatchHistoryFilter* m_filter;
};

}  // namespace PotatoAlert::Gui
