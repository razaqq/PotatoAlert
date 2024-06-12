// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Core/File.hpp"
#include "Core/Flags.hpp"

#include <cstdint>
#include <string>
#include <utility>


namespace PotatoAlert::Core {

class FileMapping
{
public:
	enum class Handle : uintptr_t
	{
		Null = 0
	};

	enum class Flags : uint32_t
	{
		Read    = 0x1,
		Write   = 0x2,
		Execute = 0x4,
		None    = 0x8
	};

	FileMapping() : m_handle(Handle::Null) {}

	explicit FileMapping(Handle handle) : m_handle(handle) {}

	FileMapping(FileMapping&& src) noexcept
	{
		m_handle = std::exchange(src.m_handle, Handle::Null);
	}

	FileMapping(const FileMapping&) = delete;

	FileMapping& operator=(FileMapping&& src) noexcept
	{
		if (m_handle != Handle::Null)
			RawClose(m_handle);
		m_handle = std::exchange(src.m_handle, Handle::Null);
		return *this;
	}

	FileMapping& operator=(const FileMapping&) = delete;

	~FileMapping()
	{
		if (m_handle != Handle::Null)
			RawClose(m_handle);
	}

	[[nodiscard]] Handle GetHandle() const
	{
		return m_handle;
	}

	static FileMapping Open(const File& file, Flags flags, uint64_t maxSize)
	{
		return FileMapping(RawOpen(file.GetHandle(), flags, maxSize));
	}

	void Close()
	{
		RawClose(std::exchange(m_handle, Handle::Null));
	}

	void* Map(Flags flags, uint64_t offset, size_t size) const
	{
		return RawMap(m_handle, flags, offset, size);
	}

	void Unmap(const void* view, size_t size) const
	{
		return RawUnmap(m_handle, view, size);
	}

	[[nodiscard]] static std::string LastError();

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

	static Handle RawOpen(File::Handle file, Flags flags, uint64_t maxSize);
	static void RawClose(Handle handle);
	static void* RawMap(Handle handle, Flags flags, uint64_t offset, size_t size);
	static void RawUnmap(Handle handle, const void* view, size_t size);
};
DEFINE_FLAGS(FileMapping::Flags);

}  // namespace PotatoAlert::Core
