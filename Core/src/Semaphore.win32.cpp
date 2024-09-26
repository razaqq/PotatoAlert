// Copyright 2022 <github.com/razaqq>

#include "Core/Log.hpp"
#include "Core/Semaphore.hpp"

#include "win32.h"


using PotatoAlert::Core::Semaphore;

namespace {

template<typename T>
static constexpr T CreateHandle(HANDLE handle)
{
	return static_cast<T>(reinterpret_cast<uintptr_t>(handle) + 1);
}

template<typename T>
static constexpr T UnwrapHandle(Semaphore::Handle handle)
{
	return reinterpret_cast<T>(static_cast<uintptr_t>(handle) - 1);
}

}

std::string Semaphore::RawLastError()
{
	DWORD err = ::GetLastError();
	if (err == 0)
	{
		return "";
	}

	LPSTR lpMsgBuf;
	DWORD size = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&lpMsgBuf),
		0, nullptr);
	
	std::string msg(lpMsgBuf, size);
	LocalFree(lpMsgBuf);

	return msg;
}

Semaphore::Handle Semaphore::RawCreate(std::string_view name, int initialValue)
{
	if (name.size() > MAX_PATH)
	{
		return Handle::Null;
	}

	const HANDLE hSem = CreateSemaphoreA(nullptr, initialValue, initialValue + 1, name.data());

	if (hSem == nullptr)
	{
		return Handle::Null;
	}

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hSem);
		return Handle::Null;
	}

	return CreateHandle<Handle>(hSem);
}

Semaphore::Handle Semaphore::RawOpen(std::string_view name)
{
	const HANDLE hSem = OpenSemaphoreA(SYNCHRONIZE, false, name.data());

	if (hSem == nullptr)
	{
		return Handle::Null;
	}

	return CreateHandle<Handle>(hSem);
}

bool Semaphore::RawRemove([[maybe_unused]] std::string_view name)
{
	// there is no way to do this in windows
	return true;
}

bool Semaphore::RawTryLock(Handle handle)
{
	return WaitForSingleObject(UnwrapHandle<HANDLE>(handle), 0) == WAIT_OBJECT_0;
}

bool Semaphore::RawUnlock(Handle handle)
{
	return ReleaseSemaphore(UnwrapHandle<HANDLE>(handle), 1, nullptr);
}

bool Semaphore::RawClose(Handle handle)
{
	return CloseHandle(UnwrapHandle<HANDLE>(handle));
}

bool Semaphore::RawIsLocked(Handle handle)
{
	if (WaitForSingleObject(UnwrapHandle<HANDLE>(handle), 0) == WAIT_OBJECT_0)
	{
		RawUnlock(handle);
		return false;
	}
	return true;
}

bool Semaphore::RawWait(Handle handle)
{
	return WaitForSingleObject(UnwrapHandle<HANDLE>(handle), INFINITE) == WAIT_OBJECT_0;
}

bool Semaphore::RawWaitTimed(Handle handle, uint32_t timeout)
{
	return WaitForSingleObject(UnwrapHandle<HANDLE>(handle), timeout) == WAIT_OBJECT_0;
}
