// Copyright 2021 <github.com/razaqq>

#include "Zip.hpp"
#include "zip.h"
#include <optional>
#include <string>
#include <vector>


using PotatoAlert::Zip;

typedef struct zip_t* ZIP_HANDLE;

template<typename T>
static constexpr T CreateHandle(ZIP_HANDLE handle)
{
	return static_cast<T>(reinterpret_cast<uintptr_t>(handle));
}

template<typename T>
static constexpr T UnwrapHandle(Zip::Handle handle)
{
	return reinterpret_cast<T>(static_cast<uintptr_t>(handle));
}

Zip Zip::Open(std::string_view path, int compressionLevel, Mode mode)
{
	const std::string pathString(path);
	ZIP_HANDLE zip = zip_open(pathString.c_str(), compressionLevel, static_cast<char>(mode));

	if (zip != nullptr)
	{
		return Zip(CreateHandle<Handle>(zip));
	}
	return Zip();
}

void Zip::Close()
{
	zip_close(UnwrapHandle<ZIP_HANDLE>(std::exchange(m_handle, Handle::Null)));
}

bool Zip::OpenEntry(std::string_view name) const
{
	return zip_entry_open(UnwrapHandle<ZIP_HANDLE>(m_handle), std::string(name).c_str()) == 0;
}

bool Zip::OpenEntry(int index) const
{
	return zip_entry_openbyindex(UnwrapHandle<ZIP_HANDLE>(m_handle), index) == 0;
}

bool Zip::CloseEntry() const
{
	return zip_entry_close(UnwrapHandle<ZIP_HANDLE>(m_handle)) == 0;
}

std::optional<std::string> Zip::EntryName() const
{
	if (const char* entryName = zip_entry_name(UnwrapHandle<ZIP_HANDLE>(m_handle)))
		return std::string(entryName);
	return std::nullopt;
}

std::optional<int> Zip::EntryIndex() const
{
	const int res = zip_entry_index(UnwrapHandle<ZIP_HANDLE>(m_handle));
	if (res > 0)
		return res;
	return std::nullopt;
}

std::optional<bool> Zip::EntryIsDir() const
{
	const int isDir = zip_entry_isdir(UnwrapHandle<ZIP_HANDLE>(m_handle));
	if (isDir > 0)
		return static_cast<bool>(isDir);
	return std::nullopt;
}

bool Zip::WriteEntry(std::string_view entryName, const std::string& data) const
{
	auto zip = UnwrapHandle<ZIP_HANDLE>(m_handle);
	const std::string entryNameString(entryName);

	if (!zip_entry_open(zip, entryNameString.c_str()))
	{
		int success = zip_entry_write(zip, data.c_str(), data.size());
		zip_entry_close(zip);
		return success == 0;
	}
	return false;
}

void Test()
{
	zip_entry_crc32()
}

bool Zip::Create(std::string_view file, const std::vector<std::string_view>& fileNames)
{
	std::vector<const char*> arr;
	arr.reserve(fileNames.size());

	for (auto fileName : fileNames)
	{
		arr.push_back(std::string(fileName).c_str());
	}
	return zip_create(std::string(file).c_str(), &arr[0], arr.size()) == 0;
}

bool Zip::RawExtract(const char* file, const char* dir, int (*callback)(const char* fileName, void* context), void* context)
{
	return zip_extract(file, dir, callback, context) == 0;
}

uint64_t Zip::SizeEntry(std::string_view entryName) const
{
	return zip_entry_size(UnwrapHandle<ZIP_HANDLE>(m_handle));
}

int Zip::EntryCount() const
{
	return zip_total_entries(UnwrapHandle<ZIP_HANDLE>(m_handle));
}
