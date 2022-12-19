// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Client/DatabaseManager.hpp"
#include "Client/ServiceProvider.hpp"
#include "Client/StatsParser.hpp"

#include <QApplication>
#include <QAbstractTableModel>
#include <QString>
#include <QVariant>

#include <vector>


namespace PotatoAlert::Gui {

class MatchHistoryModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	explicit MatchHistoryModel(const Client::ServiceProvider& serviceProvider, QObject* parent = nullptr);

	[[nodiscard]] int columnCount(const QModelIndex& parent) const override
	{
		return m_columnCount;
	}

	[[nodiscard]] int rowCount(const QModelIndex& parent) const override
	{
		return static_cast<int>(m_matches.size());
	}

	[[nodiscard]] size_t MatchCount() const
	{
		return static_cast<int>(m_matches.size());
	}

	[[nodiscard]] const Client::Match& GetMatch(size_t idx) const
	{
		return m_matches[idx];
	}

	void DeleteMatch(size_t idx)
	{
		m_matches.erase(m_matches.begin() + idx);
	}

	void AddMatch(const Client::Match& match)
	{
		m_matches.push_back(match);
	}

	void SetMatches(std::vector<Client::Match> matches)
	{
		m_matches = matches;
	}

	void SetReplaySummary(uint32_t id, const ReplaySummary& summary);

	static std::time_t GetMatchTime(const Client::Match& match);

	[[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
	[[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	static constexpr int ButtonColumn()
	{
		return m_columnCount - 1;
	}

private:
	static constexpr int m_columnCount = 8;
	std::vector<Client::Match> m_matches;
	const Client::ServiceProvider& m_services;
};

}  // namespace PotatoAlert::Gui
