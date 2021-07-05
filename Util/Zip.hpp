// Copyright 2021 <github.com/razaqq>
#pragma once

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>


namespace PotatoAlert
{

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

	static Zip Open(std::string_view path, int compressionLevel = 5, Mode mode = Mode::Read);
	void Close();

	bool OpenEntry(std::string_view name) const;
	bool OpenEntry(int index) const;
	bool CloseEntry() const;
	[[nodiscard]] std::optional<std::string> EntryName() const;
	[[nodiscard]] std::optional<int> EntryIndex() const;
	[[nodiscard]] std::optional<bool> EntryIsDir() const;

	bool WriteEntry(std::string_view entryName, const std::string& data) const;

	[[nodiscard]] uint64_t SizeEntry(std::string_view entryName) const;
	[[nodiscard]] int EntryCount() const;
	
	static bool Extract(std::string_view file, std::string_view dir, auto callback)
	{
		return RawExtract(
			std::string(file).c_str(), std::string(dir).c_str(), 
			[](const char* entry, void* ctx) -> int { return (*static_cast<decltype(callback)*>(ctx))(entry); },
			&callback
		);
	}
	static bool Create(std::string_view file, const std::vector<std::string_view>& fileNames);

private:
	static bool RawExtract(const char* file, const char* dir, int (*callback)(const char* fileName, void* context), void* context);
	Handle m_handle;
};

}
