// Copyright 2022 <github.com/razaqq>
#pragma once

#include <string>
#include <utility>


namespace PotatoAlert::Core {

class Semaphore
{
public:
	enum class Handle : uintptr_t
	{
		Null = 0
	};

	explicit Semaphore(Handle handle) : m_handle(handle) {}

	Semaphore(Semaphore&& src) noexcept
	{
		m_handle = std::exchange(src.m_handle, Handle::Null);
	}

	Semaphore(const Semaphore&) = delete;

	Semaphore& operator=(Semaphore&& src) noexcept
	{
		if (m_handle != Handle::Null)
			RawClose(m_handle);
		m_handle = std::exchange(src.m_handle, Handle::Null);
		return *this;
	}

	Semaphore& operator=(const Semaphore&) = delete;

	static Semaphore Create(std::string_view name, int initialValue = 0)
	{
		return Semaphore(RawCreate(name, initialValue));
	}

	static Semaphore Open(std::string_view name)
	{
		return Semaphore(RawOpen(name));
	}

	[[nodiscard]] static std::string LastError()
	{
		return RawLastError();
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

	bool Wait() const
	{
		return RawWait(m_handle);
	}

	bool Wait(uint32_t timeout) const
	{
		return RawWaitTimed(m_handle, timeout);
	}

	bool Close()
	{
		return RawClose(std::exchange(m_handle, Handle::Null));
	}

	static bool Remove(std::string_view name)
	{
		return RawRemove(name);
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

	static Handle RawCreate(std::string_view name, int initialValue);
	static Handle RawOpen(std::string_view name);
	static bool RawRemove(std::string_view name);
	static bool RawWait(Handle handle);
	static bool RawWaitTimed(Handle handle, uint32_t timeout);
	static bool RawClose(Handle handle);
	static bool RawTryLock(Handle handle);
	static bool RawUnlock(Handle handle);
	static bool RawIsLocked(Handle handle);
	static std::string RawLastError();
};

}  // namespace PotatoAlert::Core
