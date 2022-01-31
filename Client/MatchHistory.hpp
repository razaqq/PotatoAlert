// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Singleton.hpp"
#include "Core/Sqlite.hpp"
#include "ReplayParser/ReplayParser.hpp"
#include "StatsParser.hpp"

#include <QString>

#include <optional>
#include <string>
#include <vector>


using PotatoAlert::ReplayParser::ReplaySummary;

namespace PotatoAlert::Client {

class MatchHistory
{
public:
	PA_SINGLETON(MatchHistory);

	struct Entry
	{
		uint32_t Id;
		std::string Hash;
		std::string ReplayName;
		std::string Date;
		std::string Ship;
		std::string Map;
		std::string MatchGroup;
		std::string StatsMode;
		std::string Player;
		std::string Region;
		std::string Json;
		std::string ArenaInfo;
		bool Analyzed = false;
		ReplaySummary ReplaySummary;
	};

	static QString GetDir();

	bool SaveMatch(const StatsParser::Match::Info& info, std::string_view arenaInfo, std::string_view hash, std::string_view json, std::string_view csv) const;
	[[nodiscard]] std::optional<StatsParser::Match> GetMatchJson(std::string_view hash) const;
	[[nodiscard]] std::optional<StatsParser::Match> GetMatchJson(uint32_t id) const;

	[[nodiscard]] std::vector<Entry> GetEntries() const;
	[[nodiscard]] std::optional<Entry> GetEntry(std::string_view hash) const;
	[[nodiscard]] std::optional<Entry> GetEntry(uint32_t id) const;
	[[nodiscard]] std::optional<Entry> GetLatestEntry() const;

	struct NonAnalyzedMatch
	{
		std::string Hash;
		std::string ReplayName;
	};

	[[nodiscard]] std::vector<NonAnalyzedMatch> GetNonAnalyzedMatches() const;
	void SetAnalyzeResult(std::string_view hash, ReplaySummary summary) const;

private:
	MatchHistory();
	~MatchHistory();

	bool WriteEntry(const Entry& entry) const;
	static Entry CreateEntry(const StatsParser::Match::Info& info, std::string_view arenaInfo, std::string_view json, std::string_view hash);

	bool WriteJson(const StatsParser::Match::Info& info, std::string_view arenaInfo, std::string_view json, std::string_view hash) const;
	static bool WriteCsv(std::string_view csv);

	void ApplyDatabaseUpdates() const;

	SQLite m_db;
};

}  // namespace PotatoAlert::Client::MatchHistory
