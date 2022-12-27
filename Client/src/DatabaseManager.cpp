// Copyright 2022 <github.com/razaqq>

#include "Client/DatabaseManager.hpp"

#include "Core/Instrumentor.hpp"
#include "Core/Preprocessor.hpp"
#include "Core/Result.hpp"
#include "Core/Sqlite.hpp"
#include "Core/String.hpp"
#include "Core/Sqlite.hpp"

#include <optional>
#include <span>
#include <string>
#include <type_traits>
#include <vector>


using PotatoAlert::Client::DatabaseManager;
using PotatoAlert::Client::Match;
using PotatoAlert::Client::NonAnalyzedMatch;
using PotatoAlert::Client::SqlResult;
using PotatoAlert::Core::SQLite;

#define PA_DB_COLUMNS_WITH_ID_X_ENTRY(Type, Name, SqlType) ", " #Name
#define PA_DB_COLUMNS_WITH_ID(Columns) "Id" Columns(PA_DB_COLUMNS_WITH_ID_X_ENTRY)

#define PA_DB_COLUMNS_TYPES_WITH_ID_X_ENTRY(Type, Name, SqlType) ", " #Name " " #SqlType
#define PA_DB_COLUMNS_TYPES_WITH_ID(Columns) "Id INTEGER PRIMARY KEY" Columns(PA_DB_COLUMNS_TYPES_WITH_ID_X_ENTRY)

#define PA_DB_COLUMNS_CHAIN_ELEM(Type, Name, SqlType) (Name)
#define PA_DB_COLUMNS_CHAIN_ELEMENS(Columns) Columns(PA_DB_COLUMNS_CHAIN_ELEM)
#define PA_DB_COLUMNS(Columns) PA_STR(PA_CHAIN_COMMA(PA_DB_COLUMNS_CHAIN_ELEMENS(Columns)))

#define PA_DB_COLUMNS_VALUES_WITH_ID_X_ENTRY(Type, Column) ", :" #Column
#define PA_DB_COLUMNS_VALUES_WITH_ID(Columns) ":Id" Columns(PA_DB_COLUMNS_VALUES_WITH_ID_X_ENTRY)

#define PA_DB_COLUMNS_VALUES_CHAIN_ELEM(Type, Name, SqlType) (:Name)
#define PA_DB_COLUMNS_VALUES_CHAIN_ELEMENS(Columns) Columns(PA_DB_COLUMNS_VALUES_CHAIN_ELEM)
#define PA_DB_COLUMNS_VALUES(Columns) PA_STR(PA_CHAIN_COMMA(PA_DB_COLUMNS_VALUES_CHAIN_ELEMENS(Columns)))

#define PA_DB_COLUMNS_VALUES_UPDATE_CHAIN_ELEM(Type, Name, SqlType) (Name = :Name)
#define PA_DB_COLUMNS_VALUES_UPDATE_CHAIN_ELEMENS(Columns) Columns(PA_DB_COLUMNS_VALUES_UPDATE_CHAIN_ELEM)
#define PA_DB_COLUMNS_VALUES_UPDATE(Columns) PA_STR(PA_CHAIN_COMMA(PA_DB_COLUMNS_VALUES_UPDATE_CHAIN_ELEMENS(Columns)))

#define PA_DB_SELECT_WITH_ID(Columns) "SELECT " PA_DB_COLUMNS_WITH_ID(Columns)
#define PA_DB_CREATE_TABLE_WITH_ID(Table, Columns) "CREATE TABLE IF NOT EXISTS " #Table " (" PA_DB_COLUMNS_TYPES_WITH_ID(Columns) ")"

namespace {

template<typename T>
static inline T ParseValue(const SQLite::Statement& stmt, int index)
{
	using Value = std::decay_t<T>;
	if constexpr (std::is_same_v<Value, bool>)
	{
		if (bool value; stmt.GetBool(index, value))
		{
			return value;
		}
	}
	else if constexpr (std::is_integral_v<Value>)
	{
		if (T value; stmt.GetInt64(index, value))
		{
			return value;
		}
	}
	else if constexpr (std::is_floating_point_v<Value>)
	{
		if (T value; stmt.GetDouble(index, value))
		{
			return value;
		}
	}
	else if constexpr (std::is_same_v<Value, std::string>)
	{
		if (std::string value; stmt.GetText(index, value))
		{
			return value;
		}
	}
	else if constexpr (std::is_same_v<Value, ReplaySummary>)
	{
		if (std::string json; stmt.GetText(index, json))
		{
			return ReplaySummary::FromJson(json);
		}
	}

	LOG_ERROR("Failed to parse value into {}", typeid(T).name());
	return T();
}

template<typename T>
static inline T GetValue(const T& value)
{
	return value;
}

static inline std::string GetValue(const ReplaySummary& value)
{
	return value.ToJson();
}

#define BIND_VALUE(Name, Stmt, Value)             \
	auto PA_CAT(_value_, Name) = GetValue(Value); \
	(Stmt).Bind(PA_STR(:Name), PA_CAT(_value_, Name))

#if 0
#define BIND_VALUE(Name, Stmt, Value)                                           \
	if constexpr (std::is_same_v<std::decay_t<decltype(Value)>, ReplaySummary>) \
	{                                                                           \
		std::string json = (Value).ToJson();                                    \
		(Stmt).Bind(Name, json);                                                \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		(Stmt).Bind(Name, Value);                                               \
	}
#endif

#if 0
template<typename  T>
static inline bool BindValue(std::string_view name, SQLite::Statement& stmt, const T& value)
{
	using Value = std::decay_t<T>;
	if constexpr (std::is_same_v<Value, ReplaySummary>)
	{
		return stmt.Bind(name, value.ToJson());
	}
	else
	{
		return stmt.Bind(name, value);
	}
}
#endif

static inline Match ParseMatch(const SQLite::Statement& stmt)
{
	int index = 0;

#define PARSE_FIELD(Type, Name, SqlType) .Name = ParseValue<Type>(stmt, index++),
	return Match{ PARSE_FIELD(uint32_t, Id, "") MATCH_FIELDS(PARSE_FIELD) };
#undef PARSE_FIELD
}

}  // namespace

DatabaseManager::DatabaseManager(SQLite& db) : m_db(db)
{
	if (!CreateTables())
	{
		LOG_ERROR("Failed to create database tables");
	}

	if (!MigrateTables())
	{
		LOG_ERROR("Failed to migrate tables");
	}
}

DatabaseManager::~DatabaseManager()
{
	if (m_db)
	{
		m_db.Close();
	}
}

SqlResult<void> DatabaseManager::CreateTables()
{
	static constexpr std::string_view createStatement = PA_DB_CREATE_TABLE_WITH_ID(matches, MATCH_FIELDS);

	if (!m_db.Execute(createStatement))
	{
		return PA_SQL_ERROR("{}", m_db.GetLastError());
	}

	return {};
}

SqlResult<void> DatabaseManager::MigrateTables()
{
	// TODO: convert the time to YYYY-MM-DD HH:MM:SS
	return {};
}

SqlResult<void> DatabaseManager::AddMatch(Match& match) const
{
	static constexpr std::string_view insertQuery = "INSERT INTO matches ("
		PA_DB_COLUMNS(MATCH_FIELDS) ") VALUES (" PA_DB_COLUMNS_VALUES(MATCH_FIELDS) ")";

	SQLite::Statement stmt(m_db, insertQuery);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}

#define BIND_VALUES(Type, Name, SqlType) BIND_VALUE(Name, stmt, match.Name);
	MATCH_FIELDS(BIND_VALUES)
#undef BIND_VALUES

	stmt.ExecuteStep();
	if (!stmt.IsDone())
	{
		return PA_SQL_ERROR("{}", m_db.GetLastError());
	}

	match.Id = m_db.GetLastRowId();

	return {};
}

SqlResult<void> DatabaseManager::DeleteMatch(std::string_view hash) const
{
	static constexpr std::string_view deleteQuery = "DELETE FROM matches WHERE Hash = :Hash";

	SQLite::Statement stmt(m_db, deleteQuery);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}

	stmt.Bind(":Hash", hash);

	stmt.ExecuteStep();
	if (!stmt.IsDone())
	{
		return PA_SQL_ERROR("{}", m_db.GetLastError());
	}

	return {};
}

SqlResult<void> DatabaseManager::DeleteMatch(uint32_t id) const
{
	static constexpr std::string_view deleteQuery = "DELETE FROM matches WHERE Id = :Id";

	SQLite::Statement stmt(m_db, deleteQuery);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}

	stmt.Bind(":Id", id);

	stmt.ExecuteStep();
	if (!stmt.IsDone())
	{
		return PA_SQL_ERROR("{}", m_db.GetLastError());
	}

	return {};
}

SqlResult<void> DatabaseManager::DeleteMatches(std::span<uint32_t> ids) const
{
	std::stringstream ss;
	std::ranges::copy(ids, std::ostream_iterator<int>(ss, ", "));
	const std::string idString = ss.str();
	const std::string x = idString.substr(0, idString.size() - 2);
	std::string deleteQuery = std::format("DELETE FROM matches WHERE Id IN ({})", x);

	SQLite::Statement stmt(m_db, deleteQuery);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}

	stmt.ExecuteStep();
	if (!stmt.IsDone())
	{
		return PA_SQL_ERROR("{}", m_db.GetLastError());
	}

	return {};
}

SqlResult<std::optional<Match>> DatabaseManager::GetMatch(std::string_view hash) const
{
	static constexpr std::string_view selectQuery =
			PA_DB_SELECT_WITH_ID(MATCH_FIELDS) " FROM matches WHERE Hash = :Hash";

	SQLite::Statement stmt(m_db, selectQuery);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}

	stmt.Bind(":Hash", hash);

	stmt.ExecuteStep();
	if (stmt.HasRow())
	{
		return ParseMatch(stmt);
	}

	return {};
}

SqlResult<std::optional<Match>> DatabaseManager::GetMatch(uint32_t id) const
{
	static constexpr std::string_view selectQuery =
			PA_DB_SELECT_WITH_ID(MATCH_FIELDS) " FROM matches WHERE Id = :Id";

	SQLite::Statement stmt(m_db, selectQuery);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}

	stmt.Bind(":Id", id);

	stmt.ExecuteStep();
	if (stmt.HasRow())
	{
		return ParseMatch(stmt);
	}

	return {};
}

SqlResult<std::vector<Match>> DatabaseManager::GetMatches() const
{
	PA_PROFILE_FUNCTION();

	std::vector<Match> matches;

	static constexpr std::string_view selectQuery =
			PA_DB_SELECT_WITH_ID(MATCH_FIELDS) " FROM matches";

	SQLite::Statement stmt(m_db, selectQuery);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}

	while (!stmt.IsDone())
	{
		stmt.ExecuteStep();
		if (stmt.HasRow())
		{
			matches.emplace_back(ParseMatch(stmt));
		}
	}

	return matches;
}

SqlResult<void> DatabaseManager::UpdateMatch(uint32_t id, const Match& match) const
{
	static constexpr std::string_view updateStatement = "UPDATE matches SET " PA_DB_COLUMNS_VALUES_UPDATE(MATCH_FIELDS) " WHERE Id = :Id";

	SQLite::Statement stmt(m_db, updateStatement);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}

	stmt.Bind(":Id", id);
#define BIND_VALUES(Type, Name, SqlType) BIND_VALUE(Name, stmt, match.Name);
	MATCH_FIELDS(BIND_VALUES)
#undef BIND_VALUES

	stmt.ExecuteStep();
	if (!stmt.IsDone())
	{
		return PA_SQL_ERROR("{}", m_db.GetLastError());
	}

	return {};
}

SqlResult<void> DatabaseManager::UpdateMatch(std::string_view hash, const Match& match) const
{
	static constexpr std::string_view updateStatement = "UPDATE matches SET " PA_DB_COLUMNS_VALUES_UPDATE(MATCH_FIELDS) " WHERE Hash = :Hash";

	SQLite::Statement stmt(m_db, updateStatement);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}

	stmt.Bind(":Hash", hash);
#define BIND_VALUES(Type, Name, SqlType) BIND_VALUE(Name, stmt, match.Name);
	MATCH_FIELDS(BIND_VALUES)
#undef BIND_VALUES

	stmt.ExecuteStep();
	if (!stmt.IsDone())
	{
		return PA_SQL_ERROR("{}", m_db.GetLastError());
	}

	return {};
}

SqlResult<void> DatabaseManager::SetMatchNonAnalyzed(uint32_t id) const
{
	static constexpr std::string_view updateStatement = "UPDATE matches SET Analyzed = false WHERE Id = :Id";

	SQLite::Statement stmt(m_db, updateStatement);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}

	stmt.Bind(":Id", id);

	stmt.ExecuteStep();
	if (!stmt.IsDone())
	{
		return PA_SQL_ERROR("{}", m_db.GetLastError());
	}

	return {};
}

SqlResult<void> DatabaseManager::SetMatchNonAnalyzed(std::string_view hash) const
{
	static constexpr std::string_view updateStatement = "UPDATE matches SET Analyzed = false WHERE Hash = :Hash";

	SQLite::Statement stmt(m_db, updateStatement);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}

	stmt.Bind(":Hash", hash);

	stmt.ExecuteStep();
	if (!stmt.IsDone())
	{
		return PA_SQL_ERROR("{}", m_db.GetLastError());
	}

	return {};
}

SqlResult<std::vector<NonAnalyzedMatch>> DatabaseManager::GetNonAnalyzedMatches() const
{
	std::vector<NonAnalyzedMatch> matches;

	static constexpr std::string_view selectQuery = "SELECT Hash, ReplayName FROM matches WHERE Analyzed = false";

	SQLite::Statement stmt(m_db, selectQuery);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}

	while (!stmt.IsDone())
	{
		stmt.ExecuteStep();
		if (stmt.HasRow())
		{
			matches.emplace_back(NonAnalyzedMatch
			{
				ParseValue<std::string>(stmt, 0),
				ParseValue<std::string>(stmt, 1)
			});
		}
	}

	return matches;
}

SqlResult<std::optional<Match>> DatabaseManager::GetLatestMatch() const
{
	static constexpr std::string_view selectQuery = PA_DB_SELECT_WITH_ID(MATCH_FIELDS) " FROM matches ORDER BY Id DESC LIMIT 1";

	SQLite::Statement stmt(m_db, selectQuery);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}

	stmt.ExecuteStep();
	if (stmt.HasRow())
	{
		return ParseMatch(stmt);
	}
	return std::nullopt;
}

SqlResult<std::optional<std::string>> DatabaseManager::GetMatchJson(uint32_t id) const
{
	static constexpr std::string_view selectQuery = "SELECT Json FROM matches WHERE Id = :Id";

	SQLite::Statement stmt(m_db, selectQuery);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}

	stmt.Bind(":Id", id);

	stmt.ExecuteStep();
	if (stmt.HasRow())
	{
		std::string json;
		if (stmt.GetText(0, json))
		{
			return json;
		}
		return PA_SQL_ERROR("Failed to get json column as string");
	}

	return {};
}

SqlResult<std::optional<std::string>> DatabaseManager::GetMatchJson(std::string_view hash) const
{
	static constexpr std::string_view selectQuery = "SELECT Json FROM matches WHERE Hash = :Hash";

	SQLite::Statement stmt(m_db, selectQuery);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}

	stmt.Bind(":Hash", hash);

	stmt.ExecuteStep();
	if (stmt.HasRow())
	{
		std::string json;
		if (stmt.GetText(0, json))
		{
			return json;
		}
		return PA_SQL_ERROR("Failed to get json column as string");
	}

	return {};
}

SqlResult<void> DatabaseManager::SetMatchReplaySummary(uint32_t id, const ReplaySummary& replaySummary) const
{
	static constexpr std::string_view updateStatement = "UPDATE matches SET Analyzed = TRUE, ReplaySummary = :ReplaySummary WHERE Id = :Id";

	SQLite::Statement stmt(m_db, updateStatement);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}
	stmt.Bind(":Id", id);
	const std::string json = replaySummary.ToJson();
	stmt.Bind(":ReplaySummary", json);

	stmt.ExecuteStep();
	if (!stmt.IsDone())
	{
		return PA_SQL_ERROR("Failed to set ReplaySummary: {}", m_db.GetLastError());
	}

	return {};
}

SqlResult<void> DatabaseManager::SetMatchReplaySummary(std::string_view hash, const ReplaySummary& replaySummary) const
{
	static constexpr std::string_view updateStatement = "UPDATE matches SET Analyzed = TRUE, ReplaySummary = :ReplaySummary WHERE Hash = :Hash";

	SQLite::Statement stmt(m_db, updateStatement);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}
	stmt.Bind(":Hash", hash);
	const std::string json = replaySummary.ToJson();
	stmt.Bind(":ReplaySummary", json);

	stmt.ExecuteStep();
	if (!stmt.IsDone())
	{
		return PA_SQL_ERROR("Failed to set ReplaySummary: {}", m_db.GetLastError());
	}

	return {};
}

SqlResult<bool> DatabaseManager::MatchExists(uint32_t id) const
{
	static constexpr std::string_view existsQuery = "SELECT 1 FROM matches WHERE Id = :Id";

	SQLite::Statement stmt(m_db, existsQuery);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}
	stmt.Bind(":Id", id);

	stmt.ExecuteStep();
	return stmt.HasRow();
}

SqlResult<bool> DatabaseManager::MatchExists(std::string_view hash) const
{
	static constexpr std::string_view existsQuery = "SELECT EXISTS(SELECT 1 FROM matches WHERE Hash = :Hash)";

	SQLite::Statement stmt(m_db, existsQuery);

	if (!stmt)
	{
		return PA_SQL_ERROR("Failed to prepare SQL statement: {}", m_db.GetLastError());
	}
	stmt.Bind(":Hash", hash);

	stmt.ExecuteStep();
	if (stmt.HasRow())
	{
		bool exists = false;
		if (stmt.GetBool(0, exists))
		{
			return exists;
		}
		return PA_SQL_ERROR("Result was not a bool");
	}
	return PA_SQL_ERROR("Result had no row");
}