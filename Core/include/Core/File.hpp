// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Bytes.hpp"
#include "Core/Flags.hpp"
#include "Core/Version.hpp"

#include <filesystem>
#include <span>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>


namespace PotatoAlert::Core {

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
		Append   = 0x20,
		Truncate = 0x40
	};

	File() : m_handle(Handle::Null) {}

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

	template<is_byte T>
	bool Read(std::vector<T>& out, uint64_t size, bool resetFilePointer = true) const
	{
		return RawRead(m_handle, out, size, resetFilePointer);
	}

	template<is_byte T>
	bool ReadAll(std::vector<T>& out, bool resetFilePointer = true) const
	{
		return RawReadAll(m_handle, out, resetFilePointer);
	}

	bool ReadAllString(std::string& out, uint64_t size, bool resetFilePointer = true) const
	{
		return RawReadString(m_handle, out, size, resetFilePointer);
	}

	bool ReadAllString(std::string& out, bool resetFilePointer = true) const
	{
		return RawReadAllString(m_handle, out, resetFilePointer);
	}

	template<is_byte T>
	bool Write(std::span<const T> data, bool resetFilePointer = true) const
	{
		return RawWrite(m_handle, data, resetFilePointer);
	}

	bool WriteString(std::string_view data, bool resetFilePointer = true) const
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

	bool MoveFilePointer(int64_t offset, FilePointerMoveMethod method = FilePointerMoveMethod::Begin) const
	{
		return RawMoveFilePointer(m_handle, offset, method);
	}

	[[nodiscard]] int64_t CurrentFilePointer() const
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

	[[nodiscard]] static std::string LastError()
	{
		return RawLastError();
	}

	[[nodiscard]] bool IsOpen() const
	{
		return m_handle != Handle::Null;
	}

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
	template<is_byte T>
	static bool RawRead(Handle handle, std::vector<T>& out, uint64_t size, bool resetFilePointer);
	template<is_byte T>
	static bool RawReadAll(Handle handle, std::vector<T>& out, bool resetFilePointer);
	static bool RawReadString(Handle handle, std::string& out, uint64_t size, bool resetFilePointer);
	static bool RawReadAllString(Handle handle, std::string& out, bool resetFilePointer);
	template<is_byte T>
	static bool RawWrite(Handle handle, std::span<const T> data, bool resetFilePointer);
	static bool RawWriteString(Handle handle, std::string_view data, bool resetFilePointer);
	static bool RawFlushBuffer(Handle handle);
	static uint64_t RawGetSize(Handle handle);
	static Handle RawOpen(std::string_view path, Flags flags);
	static void RawClose(Handle handle);
	static bool RawMove(std::string_view src, std::string_view dst);
	static bool RawDelete(std::string_view file);
	static bool RawExists(std::string_view file);
	static bool RawMoveFilePointer(Handle handle, int64_t offset, FilePointerMoveMethod method);
	static int64_t RawCurrentFilePointer(Handle handle);
	static std::string RawLastError();
};
DEFINE_FLAGS(File::Flags)

}  // namespace PotatoAlert::Core
