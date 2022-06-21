// Copyright 2022 <github.com/razaqq>

#include "Core/File.hpp"
#include "Core/FileMapping.hpp"

#include <cassert>
#include <win32.h>


using PotatoAlert::Core::File;
using PotatoAlert::Core::FileMapping;

namespace {

template<typename T>
static constexpr T CreateHandle(HANDLE handle)
{
	return static_cast<T>(reinterpret_cast<uintptr_t>(handle) + 1);
}

template<typename T>
static constexpr T UnwrapHandle(FileMapping::Handle handle)
{
	return reinterpret_cast<T>(static_cast<uintptr_t>(handle) - 1);
}

template<typename T>
static constexpr T UnwrapHandle(File::Handle handle)
{
	return reinterpret_cast<T>(static_cast<uintptr_t>(handle) - 1);
}

}  // namespace

std::string FileMapping::LastError()
{
	return File::LastError();
}

FileMapping::Handle FileMapping::RawOpen(File::Handle file, Flags flags, uint64_t maxSize)
{
	DWORD protect = 0;

	switch (flags & (Flags::Read | Flags::Write))
	{
		case Flags::Read:
			protect = PAGE_READONLY;
			break;

		case Flags::Read | Flags::Write:
			protect = PAGE_READWRITE;
			break;

		default:
			return Handle::Null;
	}

	HANDLE hMapping = CreateFileMappingA(
			UnwrapHandle<HANDLE>(file), nullptr,
			protect, (maxSize >> 32), maxSize, nullptr);

	if (hMapping == nullptr)
	{
		return Handle::Null;
	}

	return CreateHandle<Handle>(hMapping);
}

void FileMapping::RawClose(Handle handle)
{
	CloseHandle(UnwrapHandle<HANDLE>(handle));
}

void* FileMapping::RawMap(Handle handle, Flags flags, uint64_t offset, size_t size)
{
	DWORD access = 0;

	if (HasFlag(flags, Flags::Read))
	{
		access |= FILE_MAP_READ;
	}

	if (HasFlag(flags, Flags::Write))
	{
		access |= FILE_MAP_WRITE;
	}

	assert(access != 0);

	return MapViewOfFile(
		UnwrapHandle<HANDLE>(handle),
		access, offset >> 32, static_cast<DWORD>(offset), size);
}

void FileMapping::RawUnmap(Handle handle, const void* view, size_t size)
{
	UnmapViewOfFile(view);
}
