// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Client/ServiceProvider.hpp"
#include "Client/StatsParser.hpp"

#include "Core/Singleton.hpp"
#include "Core/Sqlite.hpp"

#include "ReplayParser/ReplayParser.hpp"

#include <QDir>

#include <optional>
#include <string>
#include <vector>


namespace PotatoAlert::Client {

class MatchHistory
{
public:
	MatchHistory(const ServiceProvider& serviceProvider);
	~MatchHistory();

	struct Entry
	{
		uint32_t Id;
		std::string Hash;
		std::string ReplayName;
		std::string Date;
		std::string Ship;
		std::string ShipNation;
		std::string ShipClass;
		uint8_t ShipTier;
		std::string Map;
		std::string MatchGroup;
		std::string StatsMode;
		std::string Player;
		std::string Region;
		std::string Json;
		std::string ArenaInfo;
		bool Analyzed = false;
		ReplayParser::ReplaySummary ReplaySummary;
	};

	bool SaveMatch(const StatsParser::MatchType::InfoType& info, std::string_view arenaInfo, std::string_view hash, std::string_view json, std::string_view csv) const;
	[[nodiscard]] std::optional<StatsParser::MatchType> GetMatchJson(std::string_view hash) const;
	[[nodiscard]] std::optional<StatsParser::MatchType> GetMatchJson(uint32_t id) const;

	[[nodiscard]] std::vector<Entry> GetEntries() const;
	[[nodiscard]] std::optional<Entry> GetEntry(std::string_view hash) const;
	[[nodiscard]] std::optional<Entry> GetEntry(uint32_t id) const;
	[[nodiscard]] std::optional<Entry> GetLatestEntry() const;

	void DeleteEntry(std::string_view hash) const;
	void DeleteEntry(uint32_t id) const;

	void SetNonAnalyzed(std::string_view hash) const;
	void SetNonAnalyzed(uint32_t id) const;

	struct NonAnalyzedMatch
	{
		std::string Hash;
		std::string ReplayName;
	};

	[[nodiscard]] std::vector<NonAnalyzedMatch> GetNonAnalyzedMatches() const;
	void SetAnalyzeResult(std::string_view hash, const ReplayParser::ReplaySummary& summary) const;

private:
	const ServiceProvider& m_services;

	bool WriteEntry(const Entry& entry) const;
	static Entry CreateEntry(const StatsParser::MatchType::InfoType& info, std::string_view arenaInfo, std::string_view json, std::string_view hash);

	bool WriteJson(const StatsParser::MatchType::InfoType& info, std::string_view arenaInfo, std::string_view json, std::string_view hash) const;
	bool WriteCsv(std::string_view csv) const;

	void ApplyDatabaseUpdates() const;

	Core::SQLite m_db;
};

}  // namespace PotatoAlert::Client::MatchHistory
