// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Flags.hpp"

#include <cstdint>
#include <string>
#include <utility>


namespace PotatoAlert::Core {

class SQLite
{
public:
	enum class Handle : uintptr_t
	{
		Null = 0
	};

	enum class Flags : uint32_t
	{
		ReadOnly         = 0x00000001,
		ReadWrite        = 0x00000002,

		OpenUri          = 0x00000040,
		Create           = 0x00000004,

		NoFollowSymlinks = 0x01000000,
		Memory           = 0x00000080,
		NoMutex          = 0x00008000,
		FullMutex        = 0x00010000,
		SharedCache      = 0x00020000,
		PrivateCache     = 0x00040000,
	};

	SQLite()
	{
		m_handle = Handle::Null;
	}

	explicit SQLite(Handle handle) : m_handle(handle) {}

	SQLite(SQLite&& src) noexcept
	{
		m_handle = std::exchange(src.m_handle, Handle::Null);
	}

	SQLite(const SQLite&) = delete;

	SQLite& operator=(SQLite&& src) noexcept
	{
		if (m_handle != Handle::Null)
			RawClose(m_handle);
		m_handle = std::exchange(src.m_handle, Handle::Null);
		return *this;
	}

	SQLite& operator=(const SQLite&) = delete;

	~SQLite()
	{
		if (m_handle != Handle::Null)
			RawClose(m_handle);
	}

	[[nodiscard]] Handle GetHandle() const
	{
		return m_handle;
	}

	static SQLite Open(std::string_view path, Flags flags)
	{
		return SQLite(RawOpen(path, flags));
	}

	void Close()
	{
		RawClose(std::exchange(m_handle, Handle::Null));
	}

	bool FlushBuffer() const
	{
		return RawFlushBuffer(m_handle);
	}

	[[nodiscard]] std::string GetLastError() const;
	[[nodiscard]] int64_t GetLastRowId() const;

	bool Execute(std::string_view sql) const
	{
		return RawExecute(m_handle, sql, nullptr, nullptr);
	}

	bool Execute(std::string_view sql, auto callback) const
	{
		return RawExecute(
			m_handle, sql,
			[](void* ctx, int columns, char** columnText, char** columnNames) -> int
			{
				return (*static_cast<decltype(callback)*>(ctx))(columns, columnText, columnNames);
			},
			&callback);
	}

	struct Statement
	{
	public:
		Statement(const SQLite& db, std::string_view sql);
		~Statement();

		Statement(Statement&&) = delete;
		Statement(const Statement&) = delete;
		Statement& operator=(const Statement&) = delete;
		Statement&& operator=(Statement&&) = delete;
		
		bool Bind(int index, int value) const;
		bool Bind(int index, uint32_t value) const;
		bool Bind(int index, double value) const;
		bool Bind(int index, const char* value) const;
		bool Bind(int index, const std::string& value) const;
		bool Bind(int index, std::string_view value) const;

		bool Bind(std::string_view name, int value) const;
		bool Bind(std::string_view name, uint32_t value) const;
		bool Bind(std::string_view name, double value) const;
		bool Bind(std::string_view name, const char* value) const;
		bool Bind(std::string_view name, const std::string& value) const;
		bool Bind(std::string_view name, std::string_view value) const;

		bool GetText(int index, std::string& outStr) const;
		template<typename T>
		bool GetInt(int index, T& outInt) const;
		template<typename T>
		bool GetInt64(int index, T& outInt) const;
		bool GetBool(int index, bool& outBool) const;
		bool GetDouble(int index, double& outDouble) const;

		void ExecuteStep();
		[[nodiscard]] bool IsDone() const { return m_done; }
		[[nodiscard]] bool HasRow() const { return m_hasRow; }

		[[nodiscard]] const SQLite& GetDb() const { return m_db; }

		operator bool() const { return m_valid; }

	private:
		const SQLite& m_db;
		uintptr_t m_stmt;
		bool m_valid;
		bool m_done = false;
		bool m_hasRow = false;
		int m_columnCount;
	};

	explicit operator bool() const
	{
		return m_handle != Handle::Null;
	}

	bool operator==(decltype(nullptr)) const
	{
		return m_handle == Handle::Null;
	}

	bool operator!=(decltype(nullptr)) const
	{
		return m_handle != Handle::Null;
	}

private:
	Handle m_handle;

	static Handle RawOpen(std::string_view path, Flags flags);
	static void RawClose(Handle handle);
	static bool RawFlushBuffer(Handle handle);
	static bool RawExecute(
		Handle handle, std::string_view sql,
		int (*callback)(void* data, int columns, char** columnText, char** columnNames),
		void* context);
};
DEFINE_FLAGS(SQLite::Flags);

}  // namespace PotatoAlert::Core
