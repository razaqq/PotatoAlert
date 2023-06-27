// Copyright 2022 <github.com/razaqq>

#include "Core/File.hpp"
#include "Core/FileMapping.hpp"

#include <sys/mman.h>
#include <unistd.h>

#include <cstdint>


using PotatoAlert::Core::File;
using PotatoAlert::Core::FileMapping;

namespace {

template<typename T>
static constexpr T CreateHandle(int handle)
{
	return static_cast<T>(static_cast<uintptr_t>(handle) + 1);
}

template<typename T>
static constexpr T UnwrapHandle(FileMapping::Handle handle)
{
	return static_cast<T>(static_cast<uintptr_t>(handle) - 1);
}

template<typename T>
static constexpr T UnwrapHandle(File::Handle handle)
{
	return reinterpret_cast<T>(static_cast<uintptr_t>(handle) - 1);
}

}  // namespace

std::string FileMapping::LastError()
{
	return "";
}

FileMapping::Handle FileMapping::RawOpen(File::Handle file, Flags flags, uint64_t maxSize)
{
}

void FileMapping::RawClose(Handle handle)
{
}

void* FileMapping::RawMap(Handle handle, Flags flags, uint64_t offset, size_t size)
{
	int prot = 0;

	if (HasFlag(flags, Flags::Read))
		prot |= PROT_READ;
	if (HasFlag(flags, Flags::Write))
		prot |= PROT_WRITE;
	if (HasFlag(flags, Flags::Execute))
		prot |= PROT_EXEC;
	if (HasFlag(flags, Flags::None))
		prot |= PROT_NONE;

	int shareFlag = 0;

	size_t length = 0;
	long pageSize = sysconf(_SC_PAGE_SIZE);
	void* addr = mmap(nullptr, length, prot, shareFlag, UnwrapHandle<int>(handle), offset);
}

void FileMapping::RawUnmap(Handle handle, const void* view, size_t size)
{
	if (munmap(const_cast<void*>(view), size) != 0)
	{
		// TODO: handle error
	}
}
