// Copyright 2021 <github.com/razaqq>

#include "Serializer.hpp"

#include "File.hpp"
#include "Hash.hpp"
#include "Log.hpp"
#include "Sqlite.hpp"
#include "Time.hpp"

#include <QDir>
#include <QStandardPaths>
#include <QString>

#include <format>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>


using PotatoAlert::Serializer;
namespace sp = PotatoAlert::StatsParser;
using PotatoAlert::File;


static constexpr std::string_view timeFormat = "%Y-%m-%d_%H-%M-%S";

void Serializer::ApplyDatabaseUpdates() const
{
	this->m_db.Execute("ALTER TABLE matches ADD arenaInfo TEXT;");
}

Serializer::Serializer()
{
	this->m_db = SQLite::Open(QDir(GetDir()).filePath("match_history.db").toStdString().c_str(), SQLite::Flags::ReadWrite | SQLite::Flags::Create);
	if (this->m_db)
	{
		if (!this->m_db.Execute(R"(
			CREATE TABLE IF NOT EXISTS matches
			(
				id INTEGER PRIMARY KEY,
				hash TEXT UNIQUE,
				date TEXT,
				ship TEXT,
				map TEXT,
				matchGroup TEXT,
				statsMode TEXT,
				player TEXT,
				region TEXT,
				json TEXT,
				arenaInfo TEXT
			);
		)"))
		{
			LOG_ERROR("Failed to check/create table for match history database: {}", this->m_db.GetLastError());
		}

		ApplyDatabaseUpdates();
	}

	this->BuildHashSet();
}

Serializer::~Serializer()
{
	if (this->m_db)
	{
		if (!this->m_db.FlushBuffer())
			LOG_ERROR("Failed to flush buffer for match history database: {}", this->m_db.GetLastError());
		this->m_db.Close();
	}
}

QString Serializer::GetDir()
{
	QString path = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).append("/PotatoAlert/Matches");
	QDir(path).mkdir(".");
	return path;
}

static std::string GetFilePath()
{
	return QDir(Serializer::GetDir()).filePath(QString::fromStdString(std::format("match_{}.csv", PotatoAlert::Time::GetTimeStamp(timeFormat)))).toStdString();
}

bool Serializer::WriteJson(const StatsParser::Match::Info& info, std::string_view arenaInfo, std::string_view json, std::string_view hash) const
{
	if (!this->m_db)
		return false;

	LOG_TRACE("Writing match into match history database.");
	auto statement = SQLite::Statement(this->m_db, "INSERT INTO matches (hash, date, ship, map, matchGroup, statsMode, player, region, json, arenaInfo) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10);");
	
	if (!statement.Bind(1, hash))
	{
		LOG_ERROR("Failed to bind to sql statement for match history database: {}", this->m_db.GetLastError());
		return false;
	}

	if (!statement.Bind(2, info.dateTime))
	{
		LOG_ERROR("Failed to bind to sql statement for match history database: {}", this->m_db.GetLastError());
		return false;
	}

	if (!statement.Bind(3, info.ship))
	{
		LOG_ERROR("Failed to bind to sql statement for match history database: {}", this->m_db.GetLastError());
		return false;
	}

	if (!statement.Bind(4, info.map))
	{
		LOG_ERROR("Failed to bind to sql statement for match history database: {}", this->m_db.GetLastError());
		return false;
	}

	if (!statement.Bind(5, info.matchGroup))
	{
		LOG_ERROR("Failed to bind to sql statement for match history database: {}", this->m_db.GetLastError());
		return false;
	}

	if (!statement.Bind(6, info.statsMode))
	{
		LOG_ERROR("Failed to bind to sql statement for match history database: {}", this->m_db.GetLastError());
		return false;
	}

	if (!statement.Bind(7, info.player))
	{
		LOG_ERROR("Failed to bind to sql statement for match history database: {}", this->m_db.GetLastError());
		return false;
	}

	if (!statement.Bind(8, info.region))
	{
		LOG_ERROR("Failed to bind to sql statement for match history database: {}", this->m_db.GetLastError());
		return false;
	}
	
	if (!statement.Bind(9, json))
	{
		LOG_ERROR("Failed to bind to sql statement for match history database: {}", this->m_db.GetLastError());
		return false;
	}

	if (!statement.Bind(10, arenaInfo))
	{
		LOG_ERROR("Failed to bind to sql statement for match history database: {}", this->m_db.GetLastError());
		return false;
	}

	statement.ExecuteStep();
	if (!statement.IsDone())
	{
		LOG_ERROR("Failed to execute sql statement for match history database: {}", this->m_db.GetLastError());
		return false;
	}
	
	this->m_db.FlushBuffer();
	return true;
}

std::vector<Serializer::MatchHistoryEntry> Serializer::GetEntries() const
{
	if (!this->m_db)
		return {};

	LOG_TRACE("Getting entries from match history database.");
	std::vector<MatchHistoryEntry> matches;

	auto statement = SQLite::Statement(this->m_db, "SELECT id, hash, date, ship, map, matchGroup, statsMode, player, region FROM matches;");

	while (!statement.IsDone())
	{
		statement.ExecuteStep();
		if (statement.HasRow())
		{
			MatchHistoryEntry entry;
			statement.GetInt(0, entry.id);
			statement.GetText(1, entry.hash);
			statement.GetText(2, entry.date);
			statement.GetText(3, entry.ship);
			statement.GetText(4, entry.map);
			statement.GetText(5, entry.matchGroup);
			statement.GetText(6, entry.statsMode);
			statement.GetText(7, entry.player);
			statement.GetText(8, entry.region);
			matches.emplace_back(entry);
		}
	}

	return matches;
}

std::optional<Serializer::MatchHistoryEntry> Serializer::GetLatest() const
{
	if (!this->m_db)
		return {};

	LOG_TRACE("Getting latest entry from match history database.");
	auto statement = SQLite::Statement(this->m_db, "SELECT id, hash, date, ship, map, matchGroup, statsMode, player, region FROM matches ORDER BY id DESC LIMIT 1;");

	while (!statement.IsDone())
	{
		statement.ExecuteStep();
		if (statement.HasRow())
		{
			MatchHistoryEntry entry;
			statement.GetInt(0, entry.id);
			statement.GetText(1, entry.hash);
			statement.GetText(2, entry.date);
			statement.GetText(3, entry.ship);
			statement.GetText(4, entry.map);
			statement.GetText(5, entry.matchGroup);
			statement.GetText(6, entry.statsMode);
			statement.GetText(7, entry.player);
			statement.GetText(8, entry.region);
			return entry;
		}
	}

	return {};
}

std::optional<sp::Match> Serializer::GetMatch(int id) const
{
	if (!this->m_db)
		return {};

	LOG_TRACE("Getting match from match history database with id {}.", id);
	
	auto statement = SQLite::Statement(this->m_db, "SELECT json FROM matches WHERE id = ?1;");
	if (!statement.Bind(1, id))
	{
		LOG_ERROR("Failed to bind to sql statement for match history database: {}", this->m_db.GetLastError());
		return {};
	}

	statement.ExecuteStep();
	if (!statement.HasRow())
	{
		LOG_ERROR("There is no match in match history database with id: {}", id);
		return {};
	}

	std::string raw;
	if (statement.GetText(0, raw))
	{
		auto res = sp::ParseMatch(raw, false);
		if (res.success)
		{
			return res.match;
		}
		else
		{
			return {};
		}
	}
	else
	{
		LOG_ERROR("Failed to get json for match in match history database with id: {}", id);
		return {};
	}
}

bool Serializer::WriteCsv(std::string_view csv)
{
	if (File file = File::Open(GetFilePath(), File::Flags::Write | File::Flags::Create))
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

bool Serializer::SaveMatch(const StatsParser::Match::Info& info, std::string_view arenaInfo, std::string_view json, std::string_view csv)
{
	const std::string hash = HashString(arenaInfo);
	if (m_hashes.contains(hash))
	{
		LOG_TRACE("Match with hash {} is already in database.", hash);
		return false;
	}
	m_hashes.insert(hash);
	
	// save json to sqlite
	WriteJson(info, arenaInfo, json, hash);

	// save csv to file
	WriteCsv(csv);

	return true;
}

void Serializer::BuildHashSet()
{
	if (!this->m_db)
		return;

	LOG_TRACE("Building hash set for match history database.");

	auto statement = SQLite::Statement(this->m_db, "SELECT hash FROM matches;");

	while (!statement.IsDone())
	{
		statement.ExecuteStep();
		if (statement.HasRow())
		{
			std::string hash;
			statement.GetText(0, hash);
			m_hashes.insert(hash);
		}
	}
}
