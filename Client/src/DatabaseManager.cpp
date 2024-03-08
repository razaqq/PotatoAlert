// Copyright 2022 <github.com/razaqq>

#include "Client/DatabaseManager.hpp"

#include "Core/Format.hpp"
#include "Core/Instrumentor.hpp"
#include "Core/Preprocessor.hpp"
#include "Core/Result.hpp"
#include "Core/Sqlite.hpp"
#include "Core/String.hpp"
#include "Core/Time.hpp"
#include "Core/Version.hpp"

#include <optional>
#include <span>
#include <string>
#include <type_traits>
#include <vector>


using PotatoAlert::Client::DatabaseManager;
using PotatoAlert::Client::Match;
using PotatoAlert::Client::NonAnalyzedMatch;
using PotatoAlert::Client::SchemaInfo;
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
		if (int64_t value; stmt.GetInt64(index, value))
		{
			return static_cast<Value>(value);
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
			ReplaySummary summary;
			FromJson(json, summary);  // TODO: handle the result
			return summary;
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

static inline std::string GetValue(const ReplaySummary& summary)
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer writer(buffer);
	ToJson(writer, summary);  // TODO: handle the result
	return buffer.GetString();
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

static inline SchemaInfo ParseSchemaInfo(const SQLite::Statement& stmt)
{
	int index = 0;

#define PARSE_FIELD(Type, Name, SqlType) .Name = ParseValue<Type>(stmt, index++),
	return SchemaInfo{ PARSE_FIELD(uint32_t, Id, "") SCHEMAINFO_FIELDS(PARSE_FIELD) };
#undef PARSE_FIELD
}

}  // namespace

DatabaseManager::DatabaseManager(SQLite& db) : m_db(db)
{
	SqlResult<void> create = CreateTables();
	if (!create)
	{
		LOG_ERROR("Failed to create database tables: {}", create.error());
	}

	SqlResult<void> migrate = MigrateTables();
	if (!migrate)
	{
		LOG_ERROR("Failed to migrate tables: {}", migrate.error());
	}
}

DatabaseManager::~DatabaseManager()
{
	if (m_db)
	{
		if (!m_db.Execute("VACUUM"))
		{
			LOG_ERROR("Failed to VACUUM database: {}", m_db.GetLastError());
		}
		m_db.Close();
	}
}

SqlResult<void> DatabaseManager::CreateTables() const
{
	static constexpr std::string_view matchesStmt = PA_DB_CREATE_TABLE_WITH_ID(matches, MATCH_FIELDS);
	if (!m_db.Execute(matchesStmt))
	{
		return PA_SQL_ERROR("Failed to create matches table: {}", m_db.GetLastError());
	}

	static constexpr std::string_view schemaStmt = PA_DB_CREATE_TABLE_WITH_ID(schemaInfo, SCHEMAINFO_FIELDS);
	if (!m_db.Execute(schemaStmt))
	{
		return PA_SQL_ERROR("Failed to create schemaInfo table: {}", m_db.GetLastError());
	}

	return {};
}

SqlResult<void> DatabaseManager::MigrateTables() const
{
	SQLite::Statement versionStmt(m_db, PA_DB_SELECT_WITH_ID(SCHEMAINFO_FIELDS) " FROM schemaInfo");
	if (!versionStmt)
	{
		return PA_SQL_ERROR("Failed to read schemaInfo version: {}", m_db.GetLastError());
	}
	versionStmt.ExecuteStep();
	Version version(0, 0);
	if (versionStmt.HasRow())
	{
		version = Version(ParseSchemaInfo(versionStmt).Version);
	}

	// backup old database before any migration
	const bool migrationNeeded = version != m_currentVersion;
	if (migrationNeeded)
	{
		std::filesystem::path dst = m_db.GetPath();
		dst.replace_filename(fmt::format("{}_{}.{}", dst.filename(), version.ToString(), dst.extension()));
		std::error_code ec;
		std::filesystem::copy_file(m_db.GetPath(), dst, std::filesystem::copy_options::overwrite_existing, ec);
		if (ec)
		{
			return PA_SQL_ERROR("Failed to create database backup: {}", ec.message());
		}
	}

	if (version < Version(1, 0))
	{
		// convert the time to YYYY-MM-DD HH:MM:SS
		SQLite::Statement stmt(m_db, PA_DB_SELECT_WITH_ID(MATCH_FIELDS) " FROM matches WHERE Date LIKE '__.__.____ __:__:__'");
		if (!stmt)
		{
			return PA_SQL_ERROR("Failed to prepare SQL migration statement: {}", m_db.GetLastError());
		}
		while (!stmt.IsDone())
		{
			stmt.ExecuteStep();
			if (stmt.HasRow())
			{
				Match match = ParseMatch(stmt);
				if (std::optional<Core::Time::TimePoint> tp = Core::Time::StrToTime(match.Date, "%d.%m.%Y %H:%M:%S"))
				{
					match.Date = Core::Time::TimeToStr(*tp, "{:%Y-%m-%d %H:%M:%S}");
					PA_TRYV(UpdateMatch(match.Id, match));
				}
				else
				{
					return PA_SQL_ERROR("Failed to perform date migration");
				}
			}
		}

		// we have to delete all matches that are not of the supported format
		SQLite::Statement checkStmt(m_db, "SELECT Id FROM matches WHERE Date NOT LIKE '____-__-__ __:__:__'");
		if (!checkStmt)
		{
			return PA_SQL_ERROR("Failed to prepare SQL migration statement: {}", m_db.GetLastError());
		}
		while (!checkStmt.IsDone())
		{
			checkStmt.ExecuteStep();
			if (checkStmt.HasRow())
			{
				int id;
				if (checkStmt.GetInt(0, id))
				{
					PA_TRYV(DeleteMatch(id));
				}
			}
		}
	}

	// set current version
	if (migrationNeeded)
	{
		SQLite::Statement schemaInsertStmt(m_db, "INSERT OR REPLACE INTO schemaInfo (" PA_DB_COLUMNS_WITH_ID(SCHEMAINFO_FIELDS) ") VALUES (1, ?)");
		const std::string versionString = m_currentVersion.ToString(".", false);
		if (!schemaInsertStmt || !schemaInsertStmt.Bind(1, versionString))
		{
			return PA_SQL_ERROR("Failed to prepare schemaInfo statement: {}", m_db.GetLastError());
		}
		schemaInsertStmt.ExecuteStep();
		if (!schemaInsertStmt.IsDone())
		{
			return PA_SQL_ERROR("{}", m_db.GetLastError());
		}
	}

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
	const std::string deleteQuery = fmt::format("DELETE FROM matches WHERE Id IN ({})", x);

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
			PA_DB_SELECT_WITH_ID(MATCH_FIELDS) " FROM matches ORDER BY Date DESC";

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

	rapidjson::StringBuffer buffer;
	rapidjson::Writer writer(buffer);
	PA_TRYV(ToJson(writer, replaySummary));
	stmt.Bind(":ReplaySummary", buffer.GetString());

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
	rapidjson::StringBuffer buffer;
	rapidjson::Writer writer(buffer);
	PA_TRYV(ToJson(writer, replaySummary));
	stmt.Bind(":ReplaySummary", buffer.GetString());

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
		return PA_SQL_ERROR("Result is not a bool");
	}
	return PA_SQL_ERROR("Result has no row");
}
