// Copyright 2021 <github.com/razaqq>
#pragma once

#include "StatsParser.hpp"
#include "Sqlite.hpp"

#include <QString>

#include <optional>
#include <string>
#include <unordered_set>
#include <vector>


namespace PotatoAlert {

class Serializer
{
public:
	Serializer(const Serializer&) = delete;
	Serializer(Serializer&&) noexcept = delete;
	Serializer& operator=(const Serializer&) = delete;
	Serializer& operator=(Serializer&&) noexcept = delete;
	
	static Serializer& Instance()
	{
		static Serializer s;
		return s;
	}
	
	static QString GetDir();

	bool SaveMatch(const StatsParser::Match::Info& info, std::string_view arenaInfo, std::string_view json, std::string_view csv);

	struct MatchHistoryEntry
	{
		int id;
		std::string hash;
		std::string date;
		std::string ship;
		std::string map;
		std::string matchGroup;
		std::string statsMode;
		std::string player;
		std::string region;
		std::string json;
	};
	[[nodiscard]] std::optional<StatsParser::Match> GetMatch(int id) const;
	[[nodiscard]] std::vector<MatchHistoryEntry> GetEntries() const;
	[[nodiscard]] std::optional<MatchHistoryEntry> GetLatest() const;

private:
	Serializer();
	~Serializer();
	bool WriteJson(const StatsParser::Match::Info& info, std::string_view arenaInfo, std::string_view json, std::string_view hash) const;
	static bool WriteCsv(std::string_view csv);

	void ApplyDatabaseUpdates() const;
	void BuildHashSet();

	SQLite m_db;
	std::unordered_set<std::string> m_hashes;
};

}  // namespace PotatoAlert::MatchHistory
