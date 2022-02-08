// Copyright 2022 <github.com/razaqq>
#pragma once

#include <string>
#include <utility>


namespace PotatoAlert::Core {

class Mutex
{
public:
	enum class Handle : uintptr_t
	{
		Null = 0
	};

	explicit Mutex(Handle handle) : m_handle(handle) {}

	Mutex(Mutex&& src) noexcept
	{
		m_handle = std::exchange(src.m_handle, Handle::Null);
	}

	Mutex(const Mutex&) = delete;

	Mutex& operator=(Mutex&& src) noexcept
	{
		if (m_handle != Handle::Null)
			RawClose(m_handle);
		m_handle = std::exchange(src.m_handle, Handle::Null);
		return *this;
	}

	Mutex& operator=(const Mutex&) = delete;

	static Mutex Create(std::string_view name, bool initiallyOwned = true)
	{
		return Mutex(RawCreate(name, initiallyOwned));
	}

	static Mutex Open(std::string_view name)
	{
		return Mutex(RawOpen(name));
	}

	std::string LastError()
	{
		return RawLastError(m_handle);
	}

	bool IsOpen() const
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

	[[nodiscard]] Handle GetHandle() const
	{
		return m_handle;
	}

	bool Wait(uint32_t timeout = 0xFFFFFFFF) const
	{
		return RawWait(m_handle, timeout);
	}

	bool Close() const
	{
		return RawClose(m_handle);
	}

	bool TryLock() const
	{
		return RawTryLock(m_handle);
	}

	bool Unlock() const
	{
		return RawUnlock(m_handle);
	}

	[[nodiscard]] bool IsLocked() const
	{
		return RawIsLocked(m_handle);
	}

private:
	Handle m_handle = Handle::Null;

	static Handle RawCreate(std::string_view name, bool initiallyOwned);
	static Handle RawOpen(std::string_view name);
	static bool RawWait(Handle handle, uint32_t timeout);
	static bool RawClose(Handle handle);
	static bool RawTryLock(Handle handle);
	static bool RawUnlock(Handle handle);
	static bool RawIsLocked(Handle handle);
	static std::string RawLastError(Handle handle);
};

}
