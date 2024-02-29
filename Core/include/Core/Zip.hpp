// Copyright 2021 <github.com/razaqq>
#pragma once

#include "Core/Encoding.hpp"
#include "Core/Result.hpp"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <utility>


namespace PotatoAlert::Core {

class Zip
{
public:
	enum class Handle : uintptr_t
	{
		Null = 0
	};

	enum class Mode : char
	{
		Read = 'r',
		Write = 'w',
		Append = 'a',
		Delete = 'd'
	};
	
	Zip()
	{
		m_handle = Handle::Null;
	}

	explicit Zip(Handle handle) : m_handle(handle) {}

	Zip(Zip&& src) noexcept
	{
		m_handle = std::exchange(src.m_handle, Handle::Null);
	}

	Zip(const Zip&) = delete;

	Zip& operator=(Zip&& src) noexcept
	{
		if (m_handle != Handle::Null)
			Close();
		m_handle = std::exchange(src.m_handle, Handle::Null);
		return *this;
	}

	Zip& operator=(const Zip&) = delete;

	~Zip()
	{
		if (m_handle != Handle::Null)
			Close();
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

	[[nodiscard]] Handle GetHandle() const
	{
		return m_handle;
	}

	static Zip Open(const std::filesystem::path& path, int compressionLevel = 5, Mode mode = Mode::Read);
	void Close();

	bool OpenEntry(std::string_view name) const;
	bool OpenEntry(int index) const;
	bool CloseEntry() const;
	[[nodiscard]] std::optional<std::string> EntryName() const;
	[[nodiscard]] std::optional<int> EntryIndex() const;
	[[nodiscard]] std::optional<bool> EntryIsDir() const;

	bool WriteEntry(std::string_view entryName, const std::string& data) const;

	[[nodiscard]] uint64_t SizeEntry() const;
	[[nodiscard]] int EntryCount() const;

	template<typename T = void>
	static bool Extract(const std::filesystem::path& file, const std::filesystem::path& dir, auto callback)
	{
		std::string filePath, dirPath;
		if constexpr (std::is_same_v<std::filesystem::path::value_type, char>)
		{
			filePath = file.string();
			dirPath = dir.string();
		}
		else
		{
			if (const Result<std::string> res = PathToUtf8(file))
				filePath = res.value();
			else
				return false;
			if (const Result<std::string> res = PathToUtf8(dir))
				dirPath = res.value();
			else
				return false;
		}
		return RawExtract(filePath.c_str(), dirPath.c_str(),
				[](const char* entry, void* ctx) -> int { return (*static_cast<decltype(callback)*>(ctx))(entry); },
				&callback);
	}

	static bool Create(std::string_view archiveName, std::span<std::string_view> fileNames);

private:
	static bool RawExtract(const char* file, const char* dir, int (*callback)(const char* fileName, void* context), void* context);
	Handle m_handle;
};

}  // namespace PotatoAlert::Core
