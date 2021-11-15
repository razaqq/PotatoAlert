// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Flags.hpp"
#include "Version.hpp"

#include <filesystem>
#include <span>
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

		NoBuffer = 0x10,
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

	template<typename T> requires(sizeof(T) == 1)
	bool Read(std::vector<T>& out, bool resetFilePointer = true) const
	{
		return RawRead(m_handle, out, resetFilePointer);
	}

	bool ReadString(std::string& out, bool resetFilePointer = true) const
	{
		return RawReadString(m_handle, out, resetFilePointer);
	}

	bool Write(std::span<const std::byte> data, bool resetFilePointer = true) const
	{
		return RawWrite(m_handle, data, resetFilePointer);
	}

	bool WriteString(const std::string& data, bool resetFilePointer = true) const
	{
		return RawWriteString(m_handle, data, resetFilePointer);
	}

	bool FlushBuffer() const
	{
		return RawFlushBuffer(m_handle);
	}

	enum class FilePointerMoveMethod
	{
		Begin,
		Current,
		End
	};

	bool MoveFilePointer(long offset, FilePointerMoveMethod method = FilePointerMoveMethod::Begin) const
	{
		return RawMoveFilePointer(m_handle, offset, method);
	}

	unsigned long CurrentFilePointer() const
	{
		return RawCurrentFilePointer(m_handle);
	}

	bool ResetFilePointer() const
	{
		return RawMoveFilePointer(m_handle, 0, FilePointerMoveMethod::Begin);
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

	static bool GetVersion(std::string_view fileName, Version& outVersion);

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
	template<typename T>
	static bool RawRead(Handle handle, std::vector<T>& out, bool resetFilePointer);
	static bool RawReadString(Handle handle, std::string& out, bool resetFilePointer);
	static bool RawWrite(Handle handle, std::span<const std::byte> data, bool resetFilePointer);
	static bool RawWriteString(Handle handle, const std::string& data, bool resetFilePointer);
	static bool RawFlushBuffer(Handle handle);
	static uint64_t RawGetSize(Handle handle);
	static Handle RawOpen(std::string_view path, Flags flags);
	static void RawClose(Handle handle);
	static bool RawMove(std::string_view src, std::string_view dst);
	static bool RawDelete(std::string_view file);
	static bool RawExists(std::string_view file);
	static bool RawMoveFilePointer(Handle handle, long offset, FilePointerMoveMethod method);
	static unsigned long RawCurrentFilePointer(Handle handle);
};
DEFINE_FLAGS(File::Flags);

}  // namespace PotatoAlert
