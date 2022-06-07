// Copyright 2021 <github.com/razaqq>

#include "Client/MatchHistory.hpp"

#include "Client/StandardPaths.hpp"
#include "Core/File.hpp"
#include "Core/Log.hpp"
#include "Core/Sqlite.hpp"
#include "Core/Time.hpp"
#include "ReplayParser/ReplayParser.hpp"

#include <QDir>
#include <QStandardPaths>
#include <QString>

#include <format>
#include <future>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>


using namespace PotatoAlert;
using namespace PotatoAlert::Core;
using ReplayParser::ReplaySummary;
using Client::StatsParser::Match;
using Client::StatsParser::StatsParseResult;
using Client::MatchHistory;
namespace sp = Client::StatsParser;


static constexpr std::string_view timeFormat = "%Y-%m-%d_%H-%M-%S";

namespace {

static std::string GetFilePath()
{
	return MatchHistory::GetDir().filePath(QString::fromStdString(std::format("match_{}.csv", Time::GetTimeStamp(timeFormat)))).toStdString();
}

static MatchHistory::Entry ReadEntry(const SQLite::Statement& statement)
{
	MatchHistory::Entry entry;
	if (!statement.GetUInt(0, entry.Id))
		LOG_ERROR("Failed to get entry.Id");
	if (!statement.GetText(1, entry.Hash))
		LOG_ERROR("Failed to get entry.Hash");
	if (!statement.GetText(2, entry.ReplayName))
		LOG_ERROR("Failed to get entry.ReplayName");
	if (!statement.GetText(3, entry.Date))
		LOG_ERROR("Failed to get entry.Date");
	if (!statement.GetText(4, entry.Ship))
		LOG_ERROR("Failed to get entry.Ship");
	if (!statement.GetText(5, entry.Map))
		LOG_ERROR("Failed to get entry.Map");
	if (!statement.GetText(6, entry.MatchGroup))
		LOG_ERROR("Failed to get entry.MatchGroup");
	if (!statement.GetText(7, entry.StatsMode))
		LOG_ERROR("Failed to get entry.StatsMode");
	if (!statement.GetText(8, entry.Player))
		LOG_ERROR("Failed to get entry.Player");
	if (!statement.GetText(9, entry.Region))
		LOG_ERROR("Failed to get entry.Region");
	std::string summary;
	if (!statement.GetText(10, summary))
		LOG_ERROR("Failed to get entry.ReplaySummary");
	entry.ReplaySummary = ReplaySummary::FromJson(summary);
	return entry;
}

}

MatchHistory::MatchHistory()
{
	m_db = SQLite::Open(GetDir().filePath("match_history.db").toStdString().c_str(), SQLite::Flags::ReadWrite | SQLite::Flags::Create);
	if (m_db)
	{
		if (!m_db.Execute(R"(
			CREATE TABLE IF NOT EXISTS matches
			(
				Id INTEGER PRIMARY KEY,
				Hash TEXT UNIQUE,
				ReplayName TEXT,
				Date TEXT,
				Ship TEXT,
				Map TEXT,
				MatchGroup TEXT,
				StatsMode TEXT,
				Player TEXT,
				Region TEXT,
				Json TEXT,
				ArenaInfo TEXT,
				Analyzed INTEGER DEFAULT FALSE,
				ReplaySummary TEXT
			);
		)"))
		{
			LOG_ERROR("Failed to check/create table for match history database: {}", m_db.GetLastError());
		}

		ApplyDatabaseUpdates();
	}

	// BuildHashSet();
}

MatchHistory::~MatchHistory()
{
	if (m_db)
	{
		if (!m_db.FlushBuffer())
			LOG_ERROR("Failed to flush buffer for match history database: {}", m_db.GetLastError());
		m_db.Close();
	}
}

QDir MatchHistory::GetDir()
{
	const fs::path matchesPath = AppDataPath() / "Matches";
	if (!fs::exists(matchesPath))
	{
		LOG_TRACE("Creating match history directory: {}", matchesPath.string());
		std::error_code ec;
		if (!fs::create_directories(matchesPath, ec))
		{
			LOG_ERROR("Failed to create match history directory: {}", ec.message());
		}
	}
	return QString::fromStdString(matchesPath.string());
}

MatchHistory::Entry MatchHistory::CreateEntry(const Match::Info& info, std::string_view arenaInfo, std::string_view json, std::string_view hash)
{
	auto dateSplit = String::Split(info.dateTime, " ");
	auto date = String::Split(dateSplit[0], ".");
	auto time = String::Split(dateSplit[1], ":");

	const std::string replayName = std::format("{}{}{}_{}{}{}_{}_{}.wowsreplay", date[2], date[1], date[0], time[0], time[1], time[2], info.shipIdent, info.map);

	return Entry
	{
		.Hash = std::string(hash),
		.ReplayName = replayName,
		.Date = info.dateTime,
		.Ship = info.ship,
		.Map = info.map,
		.MatchGroup = info.matchGroup,
		.StatsMode = info.statsMode,
		.Player = info.player,
		.Region = info.region,
		.Json = std::string(json),
		.ArenaInfo = std::string(arenaInfo),
		.Analyzed = false,
		.ReplaySummary = ReplaySummary::FromJson("{}")
	};
}

bool MatchHistory::WriteEntry(const Entry& entry) const
{
	auto statement = SQLite::Statement(m_db, 
		R"(INSERT INTO matches (Hash, ReplayName, Date, Ship, Map, MatchGroup, StatsMode, Player, Region, Json, ArenaInfo, Analyzed, ReplaySummary) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13);)");

#define BIND(index, value)                                                                                \
	if (!statement.Bind((index), (value)))                                                                \
	{                                                                                                     \
		LOG_ERROR("Failed to bind to sql statement for match history database: {}", m_db.GetLastError()); \
		return false;                                                                                     \
	}

	BIND(1, entry.Hash)
	BIND(2, entry.ReplayName)
	BIND(3, entry.Date)
	BIND(4, entry.Ship)
	BIND(5, entry.Map)
	BIND(6, entry.MatchGroup)
	BIND(7, entry.StatsMode)
	BIND(8, entry.Player)
	BIND(9, entry.Region)
	BIND(10, entry.Json)
	BIND(11, entry.ArenaInfo)
	BIND(12, entry.Analyzed)
	const std::string summary = entry.ReplaySummary.ToJson();
	BIND(13, summary)

	statement.ExecuteStep();
	if (!statement.IsDone())
	{
		LOG_ERROR("Failed to execute sql statement for match history database: {}", m_db.GetLastError());
		return false;
	}

	m_db.FlushBuffer();
	return true;
}

bool MatchHistory::WriteJson(const Match::Info& info, std::string_view arenaInfo, std::string_view json, std::string_view hash) const
{
	if (!m_db)
		return false;

	LOG_TRACE("Writing match into match history database.");

	return WriteEntry(CreateEntry(info, arenaInfo, json, hash));
}

std::vector<MatchHistory::Entry> MatchHistory::GetEntries() const
{
	if (!m_db)
		return {};

	LOG_TRACE("Getting entries from match history database.");
	std::vector<Entry> matches;

	auto statement = SQLite::Statement(m_db, "SELECT Id, Hash, ReplayName, Date, Ship, Map, MatchGroup, StatsMode, Player, Region, ReplaySummary FROM matches;");

	if (!statement)
	{
		LOG_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
		return {};
	}

	while (!statement.IsDone())
	{
		statement.ExecuteStep();
		if (statement.HasRow())
		{
			matches.emplace_back(ReadEntry(statement));
		}
	}

	return matches;
}

std::optional<MatchHistory::Entry> MatchHistory::GetEntry(std::string_view hash) const
{
	if (!m_db)
		return {};

	auto statement = SQLite::Statement(m_db, "SELECT Id, Hash, ReplayName, Date, Ship, Map, MatchGroup, StatsMode, Player, Region, ReplaySummary FROM matches WHERE Hash = ?1;");

	if (!statement)
	{
		LOG_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
		return {};
	}

	if (!statement.Bind(1, hash))
	{
		LOG_ERROR("Failed to bind to SQL statement: {}", m_db.GetLastError());
		return {};
	}

	statement.ExecuteStep();
	if (statement.HasRow())
	{
		return ReadEntry(statement);
	}

	return {};
}

std::optional<MatchHistory::Entry> MatchHistory::GetEntry(uint32_t id) const
{
	if (!m_db)
		return {};

	auto statement = SQLite::Statement(m_db, "SELECT Id, Hash, ReplayName, Date, Ship, Map, MatchGroup, StatsMode, Player, Region, ReplaySummary FROM matches WHERE Id = ?1;");

	if (!statement)
	{
		LOG_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
		return {};
	}

	if (!statement.Bind(1, id))
	{
		LOG_ERROR("Failed to bind to SQL statement: {}", m_db.GetLastError());
		return {};
	}

	statement.ExecuteStep();
	if (statement.HasRow())
	{
		Entry entry;
		statement.GetUInt(0, entry.Id);
		statement.GetText(1, entry.Hash);
		statement.GetText(2, entry.ReplayName);
		statement.GetText(3, entry.Date);
		statement.GetText(4, entry.Ship);
		statement.GetText(5, entry.Map);
		statement.GetText(6, entry.MatchGroup);
		statement.GetText(7, entry.StatsMode);
		statement.GetText(8, entry.Player);
		statement.GetText(9, entry.Region);
		std::string summary;
		statement.GetText(10, summary);
		entry.ReplaySummary = ReplaySummary::FromJson(summary);
		return entry;
	}

	return {};
}

void MatchHistory::DeleteEntry(std::string_view hash) const
{
	LOG_INFO("Deleting entry with hash '{}' from match history", hash);

	auto statement = SQLite::Statement(m_db, "DELETE FROM matches WHERE Hash = ?1");

	if (!statement)
	{
		LOG_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
		return;
	}

	if (!statement.Bind(1, hash))
	{
		LOG_ERROR("Failed to bind to SQL statement: {}", m_db.GetLastError());
		return;
	}

	statement.ExecuteStep();
	if (!statement.IsDone())
	{
		LOG_ERROR("Failed to delete match with hash '{}' from match history: {}", hash, m_db.GetLastError());
	}
}

void MatchHistory::DeleteEntry(uint32_t id) const
{
	LOG_INFO("Deleting entry with id '{}' from match history", id);

	auto statement = SQLite::Statement(m_db, "DELETE FROM matches WHERE Id = ?1");

	if (!statement)
	{
		LOG_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
		return;
	}

	if (!statement.Bind(1, id))
	{
		LOG_ERROR("Failed to bind to SQL statement: {}", m_db.GetLastError());
		return;
	}

	statement.ExecuteStep();
	if (!statement.IsDone())
	{
		LOG_ERROR("Failed to delete match with id '{}' from match history: {}", id, m_db.GetLastError());
	}
}

void MatchHistory::SetNonAnalyzed(std::string_view hash) const
{
	LOG_INFO("Setting match with hash '{}' to non-analyzed", hash);

	auto statement = SQLite::Statement(m_db, "UPDATE matches SET Analyzed = false WHERE Hash = ?1");

	if (!statement)
	{
		LOG_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
		return;
	}

	if (!statement.Bind(1, hash))
	{
		LOG_ERROR("Failed to bind to SQL statement: {}", m_db.GetLastError());
		return;
	}

	statement.ExecuteStep();
	if (!statement.IsDone())
	{
		LOG_ERROR("Failed set match with hash '{}' to non-analyzed: {}", hash, m_db.GetLastError());
	}
}

void MatchHistory::SetNonAnalyzed(uint32_t id) const
{
	LOG_INFO("Setting match with id '{}' to non-analyzed", id);

	auto statement = SQLite::Statement(m_db, "UPDATE matches SET Analyzed = false WHERE Id = ?1");

	if (!statement)
	{
		LOG_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
		return;
	}

	if (!statement.Bind(1, id))
	{
		LOG_ERROR("Failed to bind to SQL statement: {}", m_db.GetLastError());
		return;
	}

	statement.ExecuteStep();
	if (!statement.IsDone())
	{
		LOG_ERROR("Failed set match with id '{}' to non-analyzed: {}", id, m_db.GetLastError());
	}
}


std::optional<MatchHistory::Entry> MatchHistory::GetLatestEntry() const
{
	if (!m_db)
		return {};

	LOG_TRACE("Getting latest entry from match history database.");
	auto statement = SQLite::Statement(m_db, "SELECT Id, Hash, ReplayName, Date, Ship, Map, MatchGroup, StatsMode, Player, Region, ReplaySummary FROM matches ORDER BY id DESC LIMIT 1;");

	if (!statement)
	{
		LOG_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
		return {};
	}

	while (!statement.IsDone())
	{
		statement.ExecuteStep();
		if (statement.HasRow())
		{
			return ReadEntry(statement);
		}
	}

	return {};
}

std::optional<Match> MatchHistory::GetMatchJson(std::string_view hash) const
{
	if (!m_db)
		return {};

	LOG_TRACE("Getting match from match history database with hash {}", hash);
	
	auto statement = SQLite::Statement(m_db, "SELECT json FROM matches WHERE hash = ?1;");

	if (!statement)
	{
		LOG_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
		return {};
	}

	if (!statement.Bind(1, hash))
	{
		LOG_ERROR("Failed to bind to SQL statement: {}", m_db.GetLastError());
		return {};
	}

	statement.ExecuteStep();
	if (!statement.HasRow())
	{
		LOG_TRACE("There is no match in match history database with hash: {}", hash);
		return {};
	}

	std::string raw;
	if (statement.GetText(0, raw))
	{
		if (StatsParseResult res = sp::ParseMatch(raw, StatsParser::MatchContext{}, false); res.success)
			return res.match;
		return {};
	}
	else
	{
		LOG_ERROR("Failed to get json for match in match history database with hash: {}", hash);
		return {};
	}
}

std::optional<Match> MatchHistory::GetMatchJson(uint32_t id) const
{
	if (!m_db)
		return {};

	LOG_TRACE("Getting match from match history database with id {}", id);

	auto statement = SQLite::Statement(m_db, "SELECT json FROM matches WHERE id = ?1;");

	if (!statement)
	{
		LOG_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
		return {};
	}

	if (!statement.Bind(1, id))
	{
		LOG_ERROR("Failed to bind to SQL statement: {}", m_db.GetLastError());
		return {};
	}

	statement.ExecuteStep();
	if (!statement.HasRow())
	{
		LOG_TRACE("There is no match in match history database with id: {}", id);
		return {};
	}

	std::string raw;
	if (statement.GetText(0, raw))
	{
		if (StatsParseResult res = sp::ParseMatch(raw, StatsParser::MatchContext{}, false); res.success)
			return res.match;
		return {};
	}
	else
	{
		LOG_ERROR("Failed to get json for match in match history database with id: {}", id);
		return {};
	}
}

bool MatchHistory::WriteCsv(std::string_view csv)
{
	if (const File file = File::Open(GetFilePath(), File::Flags::Write | File::Flags::Create))
	{
		if (file.WriteString(csv))
		{
			LOG_TRACE("Wrote match as CSV.");
			return true;
		}
		
		LOG_ERROR("Failed to save match as csv: {}", File::LastError());
		return false;
	}

	LOG_ERROR("Failed to open csv file for writing: {}", File::LastError());
	return false;
}

bool MatchHistory::SaveMatch(const Match::Info& info, std::string_view arenaInfo, std::string_view hash, std::string_view json, std::string_view csv) const
{
	if (std::optional<Match> match = GetMatchJson(hash))
	{
		LOG_TRACE("Match with hash {} is already in database.", hash);
		return false;
	}
	
	// save json to sqlite
	WriteJson(info, arenaInfo, json, hash);

	// save csv to file
	WriteCsv(csv);

	return true;
}

void MatchHistory::ApplyDatabaseUpdates() const
{
	auto statement = SQLite::Statement(m_db, "PRAGMA table_info(matches);");

	if (!statement)
	{
		LOG_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
		return;
	}

	std::unordered_set<std::string> columns;
	while (!statement.IsDone())
	{
		statement.ExecuteStep();
		if (statement.HasRow())
		{
			std::string columnName;
			if (!statement.GetText(1, columnName))
			{
				LOG_ERROR("Failed to get column name: {}");
				return;
			}
			columns.insert(columnName);
		}
	}

#define ADD_COLUMN(name, type)                                                       \
	if (!columns.contains((name)))                                                       \
		if (!m_db.Execute(std::format("ALTER TABLE matches ADD {} {};", (name), (type)))) \
			LOG_ERROR("Failed to create column: {}", m_db.GetLastError())

	ADD_COLUMN("Id", "INTEGER PRIMARY KEY");
	ADD_COLUMN("Hash", "TEXT UNIQUE");
	ADD_COLUMN("ReplayName", "TEXT");
	ADD_COLUMN("Date", "TEXT");
	ADD_COLUMN("Ship", "TEXT");
	ADD_COLUMN("Map", "TEXT");
	ADD_COLUMN("MatchGroup", "TEXT");
	ADD_COLUMN("StatsMode", "TEXT");
	ADD_COLUMN("Player", "TEXT");
	ADD_COLUMN("Region", "TEXT");
	ADD_COLUMN("Json", "TEXT");
	ADD_COLUMN("ArenaInfo", "TEXT");
	ADD_COLUMN("Analyzed", "INTEGER DEFAULT FALSE");
	ADD_COLUMN("ReplaySummary", "TEXT");

	if (!m_db.Execute("UPDATE matches SET analyzed = FALSE WHERE analyzed IS NULL;"))
		LOG_ERROR("Failed to set analyzed = FALSE for matches: {}", m_db.GetLastError());

	m_db.FlushBuffer();
}

std::vector<MatchHistory::NonAnalyzedMatch> MatchHistory::GetNonAnalyzedMatches() const
{
	std::vector<NonAnalyzedMatch> entries;

	auto statement = SQLite::Statement(m_db, "SELECT Hash, ReplayName FROM matches WHERE analyzed = FALSE;");

	if (!statement)
	{
		LOG_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
		return entries;
	}

	while (!statement.IsDone())
	{
		statement.ExecuteStep();
		if (statement.HasRow())
		{
			NonAnalyzedMatch match;
			statement.GetText(0, match.Hash);
			statement.GetText(1, match.ReplayName);
			entries.emplace_back(std::move(match));
		}
	}

	return entries;
}

void MatchHistory::SetAnalyzeResult(std::string_view hash, ReplaySummary summary) const
{
	auto statement = SQLite::Statement(m_db, "UPDATE matches SET analyzed = TRUE, replaySummary = ?1 WHERE hash = ?2;");

	if (!statement)
	{
		LOG_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
		return;
	}

	std::string j = summary.ToJson();
	if (!statement.Bind(1, j))
	{
		LOG_ERROR("Failed to bind to SQL statement: {}", m_db.GetLastError());
		return;
	}

	if (!statement.Bind(2, hash))
	{
		LOG_ERROR("Failed to bind to SQL statement: {}", m_db.GetLastError());
		return;
	}

	statement.ExecuteStep();
	if (!statement.IsDone())
	{
		LOG_ERROR("Failed to set replay analyzer result in db: {}", m_db.GetLastError());
		return;
	}

	m_db.FlushBuffer();
}
