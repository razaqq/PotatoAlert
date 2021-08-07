// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Flags.hpp"

#include <filesystem>
#include <string>
#include <type_traits>
#include <utility>


namespace PotatoAlert {

class File
{
public:
	enum class Handle : uintptr_t
	{
		Null = 0
	};

	enum class Flags : uint32_t
	{
		Read    = 0x01,
		Write   = 0x02,

		Open    = 0x04,
		Create  = 0x08,
	};

	File()
	{
		m_handle = Handle::Null;
	}

	explicit File(Handle handle) : m_handle(handle) {}

	File(File&& src) noexcept
	{
		m_handle = std::exchange(src.m_handle, Handle::Null);
	}

	File(const File&) = delete;

	File& operator=(File&& src) noexcept
	{
		if (m_handle != Handle::Null)
			RawClose(m_handle);
		m_handle = std::exchange(src.m_handle, Handle::Null);
		return *this;
	}

	File& operator=(const File&) = delete;

	~File()
	{
		if (m_handle != Handle::Null)
			RawClose(m_handle);
	}

	[[nodiscard]] Handle GetHandle() const
	{
		return m_handle;
	}

	static File Open(std::string_view path, Flags flags)
	{
		return File(RawOpen(path, flags));
	}

	void Close()
	{
		RawClose(std::exchange(m_handle, Handle::Null));
	}

	[[nodiscard]] uint64_t Size() const
	{
		return RawGetSize(m_handle);
	}

	bool Read(std::vector<std::byte>& out) const
	{
		return RawRead(m_handle, out);
	}

	bool ReadString(std::string& out) const
	{
		return RawReadString(m_handle, out);
	}

	bool Write(const std::string& data)
	{
		return RawWrite(m_handle, data);
	}

	bool FlushBuffer()
	{
		return RawFlushBuffer(m_handle);
	}

	static bool Move(std::string_view src, std::string_view dst)
	{
		return RawMove(src, dst);
	}

	static bool Delete(std::string_view fileName)
	{
		return RawDelete(fileName);
	}

	static bool Exists(std::string_view fileName)
	{
		return RawExists(fileName);
	}

	static bool GetVersion(std::string_view fileName, std::string& outVersion);

	static std::string LastError();

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

	// these have to be implemented for each os
	static bool RawRead(Handle handle, std::vector<std::byte>& out);
	static bool RawReadString(Handle handle, std::string& out);
	static bool RawWrite(Handle handle, const std::string& data);
	static bool RawFlushBuffer(Handle handle);
	static uint64_t RawGetSize(Handle handle);
	static Handle RawOpen(std::string_view path, Flags flags);
	static void RawClose(Handle handle);
	static bool RawMove(std::string_view src, std::string_view dst);
	static bool RawDelete(std::string_view file);
	static bool RawExists(std::string_view file);
	static bool ResetFilePointer(Handle handle);
};
DEFINE_FLAGS(File::Flags);

}  // namespace PotatoAlert
