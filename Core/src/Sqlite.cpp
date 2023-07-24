// Copyright 2021 <github.com/razaqq>

#include "Core/Sqlite.hpp"

#include "sqlite3.h"

#include <cstdint>
#include <filesystem>
#include <limits>
#include <string>
#include <type_traits>


using PotatoAlert::Core::SQLite;

static SQLite::Handle CreateHandle(sqlite3* handle)
{
	return static_cast<SQLite::Handle>(reinterpret_cast<uintptr_t>(handle));
}

static sqlite3* UnwrapHandle(SQLite::Handle handle)
{
	return reinterpret_cast<sqlite3*>(static_cast<uintptr_t>(handle));
}

SQLite::Handle SQLite::RawOpen(std::string_view path, Flags flags)
{
	sqlite3* db;
	if (sqlite3_open_v2(path.data(), &db, static_cast<int>(flags), nullptr) == SQLITE_OK)
		return CreateHandle(db);
	else
		return Handle::Null;
}

SQLite::Handle SQLite::RawOpen(const std::filesystem::path& path, Flags flags)
{
	sqlite3* db;
	// TODO: this works but is very bad, please fix
	if (sqlite3_open_v2((const char*)path.u8string().c_str(), &db, static_cast<int>(flags), nullptr) == SQLITE_OK)
		return CreateHandle(db);
	else
		return Handle::Null;
}

void SQLite::RawClose(Handle handle)
{
	sqlite3_close_v2(UnwrapHandle(handle));
}

bool SQLite::RawFlushBuffer(Handle handle)
{
	return sqlite3_db_cacheflush(UnwrapHandle(handle)) == SQLITE_OK;
}

std::string SQLite::GetLastError() const
{
	return sqlite3_errmsg(UnwrapHandle(m_handle));
}

int64_t SQLite::GetLastRowId() const
{
	return sqlite3_last_insert_rowid(UnwrapHandle(m_handle));
}

bool SQLite::RawExecute(Handle handle, std::string_view sql, int (*callback)(void* ctx, int columns, char** columnText, char** columnNames), void* context)
{
	return sqlite3_exec(UnwrapHandle(handle), std::string(sql).c_str(), callback, context, nullptr) == SQLITE_OK;
}

// ----------------------------------------------

SQLite::Statement::Statement(const SQLite& db, std::string_view sql) : m_db(db)
{
	sqlite3_stmt* stmt;
	m_valid = sqlite3_prepare_v2(UnwrapHandle(db.m_handle), sql.data(), -1, &stmt, nullptr) == SQLITE_OK;
	m_stmt = reinterpret_cast<uintptr_t>(stmt);
	m_columnCount = sqlite3_column_count(stmt);
}

SQLite::Statement::~Statement()
{
	sqlite3_finalize(reinterpret_cast<sqlite3_stmt*>(m_stmt));
}


bool SQLite::Statement::Bind(int index, int value) const
{
	return sqlite3_bind_int(reinterpret_cast<sqlite3_stmt*>(m_stmt), index, value) == SQLITE_OK;
}

bool SQLite::Statement::Bind(int index, uint32_t value) const
{
	return sqlite3_bind_int(reinterpret_cast<sqlite3_stmt*>(m_stmt), index, value) == SQLITE_OK;
}

bool SQLite::Statement::Bind(int index, double value) const
{
	return sqlite3_bind_double(reinterpret_cast<sqlite3_stmt*>(m_stmt), index, value) == SQLITE_OK;
}

bool SQLite::Statement::Bind(int index, const char* value) const
{
	return sqlite3_bind_text(reinterpret_cast<sqlite3_stmt*>(m_stmt), index, value, -1, nullptr) == SQLITE_OK;
}

bool SQLite::Statement::Bind(int index, const std::string& value) const
{
	return sqlite3_bind_text(reinterpret_cast<sqlite3_stmt*>(m_stmt), index, value.c_str(), -1, nullptr) == SQLITE_OK;
}

bool SQLite::Statement::Bind(int index, std::string_view value) const
{
	return sqlite3_bind_text(reinterpret_cast<sqlite3_stmt*>(m_stmt), index, value.data(), -1, nullptr) == SQLITE_OK;
}

bool SQLite::Statement::Bind(std::string_view name, int value) const
{
	if (const int index = sqlite3_bind_parameter_index(reinterpret_cast<sqlite3_stmt*>(m_stmt), name.data()))
	{
		return Bind(index, value);
	}
	return false;
}

bool SQLite::Statement::Bind(std::string_view name, uint32_t value) const
{
	if (const int index = sqlite3_bind_parameter_index(reinterpret_cast<sqlite3_stmt*>(m_stmt), name.data()))
	{
		return Bind(index, value);
	}
	return false;
}

bool SQLite::Statement::Bind(std::string_view name, double value) const
{
	if (const int index = sqlite3_bind_parameter_index(reinterpret_cast<sqlite3_stmt*>(m_stmt), name.data()))
	{
		return Bind(index, value);
	}
	return false;
}

bool SQLite::Statement::Bind(std::string_view name, const char* value) const
{
	if (const int index = sqlite3_bind_parameter_index(reinterpret_cast<sqlite3_stmt*>(m_stmt), name.data()))
	{
		return Bind(index, value);
	}
	return false;
}

bool SQLite::Statement::Bind(std::string_view name, const std::string& value) const
{
	if (const int index = sqlite3_bind_parameter_index(reinterpret_cast<sqlite3_stmt*>(m_stmt), name.data()))
	{
		return Bind(index, value);
	}
	return false;
}

bool SQLite::Statement::Bind(std::string_view name, std::string_view value) const
{
	if (const int index = sqlite3_bind_parameter_index(reinterpret_cast<sqlite3_stmt*>(m_stmt), name.data()))
	{
		return Bind(index, value);
	}
	return false;
}

void SQLite::Statement::ExecuteStep()
{
	int ret = sqlite3_step(reinterpret_cast<sqlite3_stmt*>(m_stmt));
	switch (ret)
	{
		case SQLITE_DONE:
			m_hasRow = false;
			m_done = true;
			break;
		case SQLITE_ROW:
			m_hasRow = true;
			m_done = false;
			break;
		default:
			m_hasRow = false;
			m_done = true;
			break;
	}
}

bool SQLite::Statement::GetText(int index, std::string& outStr) const
{
	if (!m_hasRow || index < 0 || index > m_columnCount)
	{
		return false;
	}

	auto stmt = reinterpret_cast<sqlite3_stmt*>(m_stmt);
	// careful this only works if the string is all ASCII, which we know it is
	const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, index));
	outStr = std::string(text, sqlite3_column_bytes(stmt, index));
	return true;
}

template<typename T>
bool SQLite::Statement::GetInt(int index, T& outInt) const
{
	static_assert(std::is_integral_v<T>);
	if (!m_hasRow || index < 0 || index > m_columnCount)
	{
		return false;
	}

	auto stmt = reinterpret_cast<sqlite3_stmt*>(m_stmt);
	int out = sqlite3_column_int(stmt, index);

	if (out >= std::numeric_limits<T>::min() && out <= std::numeric_limits<T>::max())
	{
		outInt = static_cast<T>(out);
		return true;
	}

	return false;
}
template bool SQLite::Statement::GetInt(int, uint8_t&) const;
template bool SQLite::Statement::GetInt(int, uint16_t&) const;
template bool SQLite::Statement::GetInt(int, uint32_t&) const;
template bool SQLite::Statement::GetInt(int, int8_t&) const;
template bool SQLite::Statement::GetInt(int, int16_t&) const;
template bool SQLite::Statement::GetInt(int, int32_t&) const;

template<typename T>
bool SQLite::Statement::GetInt64(int index, T& outInt) const
{
	static_assert(std::is_integral_v<T>);
	if (!m_hasRow || index < 0 || index > m_columnCount)
	{
		return false;
	}

	auto stmt = reinterpret_cast<sqlite3_stmt*>(m_stmt);
	sqlite3_int64 out = sqlite3_column_int64(stmt, index);

	if (out >= std::numeric_limits<T>::min() && out <= std::numeric_limits<T>::max())
	{
		outInt = static_cast<T>(out);
		return true;
	}

	return false;
}
template bool SQLite::Statement::GetInt64(int, uint8_t&) const;
template bool SQLite::Statement::GetInt64(int, uint16_t&) const;
template bool SQLite::Statement::GetInt64(int, uint32_t&) const;
template bool SQLite::Statement::GetInt64(int, uint64_t&) const;
template bool SQLite::Statement::GetInt64(int, int8_t&) const;
template bool SQLite::Statement::GetInt64(int, int16_t&) const;
template bool SQLite::Statement::GetInt64(int, int32_t&) const;
template bool SQLite::Statement::GetInt64(int, int64_t&) const;

bool SQLite::Statement::GetBool(int index, bool& outBool) const
{
	if (!m_hasRow || index < 0 || index > m_columnCount)
	{
		return false;
	}

	auto stmt = reinterpret_cast<sqlite3_stmt*>(m_stmt);
	int v = sqlite3_column_int(stmt, index);
	if (v == 0)
	{
		outBool = false;
		return true;
	}
	if (v == 1)
	{
		outBool = true;
		return true;
	}
	return false;
}

bool SQLite::Statement::GetDouble(int index, double& outDouble) const
{
	if (!m_hasRow || index < 0 || index > m_columnCount)
	{
		return false;
	}

	auto stmt = reinterpret_cast<sqlite3_stmt*>(m_stmt);
	outDouble = sqlite3_column_double(stmt, index);
	return false;
}
