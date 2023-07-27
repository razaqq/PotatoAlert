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
	MatchHistorySortFilter(QObject* parent = nullptr) : QSortFilterProxyModel(parent)
	{
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
		// bool isValid = sourceParent.isValid();
		// QModelIndex realIndex = mapToSource(sourceParent);

		const std::time_t time = sourceModel()->index(sourceRow, filterKeyColumn(), sourceParent).data(Qt::UserRole).value<std::time_t>();

		return time >= m_from && time <= m_to;
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
};

}  // namespace PotatoAlert::Gui
