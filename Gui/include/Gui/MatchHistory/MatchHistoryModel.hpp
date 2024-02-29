// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Client/DatabaseManager.hpp"
#include "Client/ServiceProvider.hpp"
#include "Client/StatsParser.hpp"

#include <QApplication>
#include <QAbstractTableModel>
#include <QString>
#include <QVariant>

#include <cstddef>
#include <vector>


namespace PotatoAlert::Gui {

class MatchHistoryModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	explicit MatchHistoryModel(const Client::ServiceProvider& serviceProvider, QObject* parent = nullptr);

	[[nodiscard]] int columnCount([[maybe_unused]] const QModelIndex& index) const override
	{
		return m_columnCount;
	}

	[[nodiscard]] int rowCount([[maybe_unused]] const QModelIndex& index) const override
	{
		return static_cast<int>(m_matches.size());
	}

	[[nodiscard]] size_t MatchCount() const
	{
		return m_matches.size();
	}

	[[nodiscard]] const Client::Match& GetMatch(size_t idx) const
	{
		return m_matches[idx];
	}

	void DeleteMatch(size_t idx)
	{
		m_matches.erase(m_matches.begin() + static_cast<ptrdiff_t>(idx));
	}

	void AddMatch(const Client::Match& match)
	{
		m_matches.insert(std::ranges::upper_bound(m_matches, match, [](const Client::Match& a, const Client::Match& b)
		{
			return a.Date > b.Date;
		}), match);
	}

	std::span<const Client::Match> GetMatches() const
	{
		return m_matches;
	}

	// matches have to be sorted
	void SetMatches(std::vector<Client::Match>&& matches)
	{
		m_matches = std::move(matches);
	}

	void SetReplaySummary(uint32_t id, const ReplaySummary& summary);

	static std::time_t GetMatchTime(const Client::Match& match);

	[[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
	[[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	static constexpr int ButtonColumn()
	{
		return m_columnCount - 1;
	}

	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	int m_headerSize = 11;
	static constexpr int m_columnCount = 8;
	std::vector<Client::Match> m_matches;
	const Client::ServiceProvider& m_services;
};

}  // namespace PotatoAlert::Gui
