// Copyright 2022 <github.com/razaqq>

#include "Core/File.hpp"
#include "Core/FileMapping.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstdint>
#include <type_traits>


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
	return static_cast<FileMapping::Handle>(static_cast<std::underlying_type_t<decltype(file)>>(file));
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

	void* addr = mmap(nullptr, size, prot, MAP_PRIVATE, UnwrapHandle<int>(handle), offset);

	if (addr == MAP_FAILED)
		return nullptr;

	return addr;
}

void FileMapping::RawUnmap(Handle handle, const void* view, size_t size)
{
	if (munmap(const_cast<void*>(view), size) != 0)
	{
		// TODO: handle error
	}
}
