// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Core/Result.hpp"
#include "Core/Sqlite.hpp"

#include "ReplayParser/ReplayParser.hpp"

#include <expected>
#include <format>
#include <string>


using PotatoAlert::Core::Result;
using PotatoAlert::ReplayParser::ReplaySummary;

namespace PotatoAlert::Client {

#define MATCH_FIELDS(X)                      \
	X(std::string, Hash, TEXT UNIQUE)        \
	X(std::string, ReplayName, TEXT)         \
	X(std::string, Date, TEXT)               \
	X(std::string, Ship, TEXT)               \
	X(std::string, ShipNation, TEXT)         \
	X(std::string, ShipClass, TEXT)          \
	X(uint8_t, ShipTier, INTEGER)            \
	X(std::string, Map, TEXT)                \
	X(std::string, MatchGroup, TEXT)         \
	X(std::string, StatsMode, TEXT)          \
	X(std::string, Player, TEXT)             \
	X(std::string, Region, TEXT)             \
	X(std::string, Json, TEXT)               \
	X(std::string, ArenaInfo, TEXT)          \
	X(bool, Analyzed, INTEGER DEFAULT FALSE) \
	X(ReplaySummary, ReplaySummary, TEXT)

#define DECL_STRUCT(Type, Name, SqlType) Type Name;

struct Match
{
	uint32_t Id;
	MATCH_FIELDS(DECL_STRUCT)
};

struct NonAnalyzedMatch
{
	std::string Hash;
	std::string ReplayName;
};

using SqlError = std::string;
template<typename T>
using SqlResult = Result<T, SqlError>;
#define PA_SQL_ERROR(...) (::std::unexpected(::PotatoAlert::Client::SqlError(std::format(__VA_ARGS__))))


class DatabaseManager
{
public:
	explicit DatabaseManager(Core::SQLite& db);
	~DatabaseManager();

	SqlResult<void> CreateTables() const;
	SqlResult<void> MigrateTables() const;

	// adds the match to db and set the id
	[[nodiscard]] SqlResult<void> AddMatch(Match& match) const;
	[[nodiscard]] SqlResult<std::optional<Match>> GetMatch(uint32_t id) const;
	[[nodiscard]] SqlResult<std::optional<Match>> GetMatch(std::string_view hash) const;
	[[nodiscard]] SqlResult<std::vector<Match>> GetMatches() const;
	[[nodiscard]] SqlResult<void> DeleteMatch(uint32_t id) const;
	[[nodiscard]] SqlResult<void> DeleteMatch(std::string_view hash) const;
	[[nodiscard]] SqlResult<void> DeleteMatches(std::span<uint32_t> ids) const;
	[[nodiscard]] SqlResult<void> UpdateMatch(uint32_t id, const Match& match) const;
	[[nodiscard]] SqlResult<void> UpdateMatch(std::string_view hash, const Match& match) const;
	[[nodiscard]] SqlResult<void> SetMatchNonAnalyzed(uint32_t id) const;
	[[nodiscard]] SqlResult<void> SetMatchNonAnalyzed(std::string_view hash) const;
	[[nodiscard]] SqlResult<std::vector<NonAnalyzedMatch>> GetNonAnalyzedMatches() const;
	[[nodiscard]] SqlResult<std::optional<Match>> GetLatestMatch() const;
	[[nodiscard]] SqlResult<std::optional<std::string>> GetMatchJson(uint32_t id) const;
	[[nodiscard]] SqlResult<std::optional<std::string>> GetMatchJson(std::string_view hash) const;
	[[nodiscard]] SqlResult<void> SetMatchReplaySummary(uint32_t id, const ReplaySummary& replaySummary) const;
	[[nodiscard]] SqlResult<void> SetMatchReplaySummary(std::string_view hash, const ReplaySummary& replaySummary) const;
	[[nodiscard]] SqlResult<bool> MatchExists(uint32_t id) const;
	[[nodiscard]] SqlResult<bool> MatchExists(std::string_view hash) const;

private:
	Core::SQLite& m_db;
	static constexpr std::string_view matchTable = "matches";
};

}  // namespace PotatoAlert::Client
