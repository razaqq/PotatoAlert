// Copyright 2022 <github.com/razaqq>

#include "Core/Log.hpp"
#include "Core/Mutex.hpp"

#include "win32.h"


using PotatoAlert::Core::Mutex;

namespace {

template<typename T>
static constexpr T CreateHandle(HANDLE handle)
{
	return static_cast<T>(reinterpret_cast<uintptr_t>(handle) + 1);
}

template<typename T>
static constexpr T UnwrapHandle(Mutex::Handle handle)
{
	return reinterpret_cast<T>(static_cast<uintptr_t>(handle) - 1);
}

}

std::string Mutex::RawLastError(Handle handle)
{
	DWORD err = GetLastError();
	LPSTR lpMsgBuf;
	FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPTSTR>(&lpMsgBuf),
			0, nullptr);
	return std::string(lpMsgBuf);
}

Mutex::Handle Mutex::RawCreate(std::string_view name, bool initiallyOwned)
{
	HANDLE hMutex = CreateMutexA(nullptr, initiallyOwned, name.data());

	if (hMutex == nullptr)
	{
		return Handle::Null;
	}

	return CreateHandle<Handle>(hMutex);
}

Mutex::Handle Mutex::RawOpen(std::string_view name)
{
	HANDLE hMutex = OpenMutexA(SYNCHRONIZE, false, name.data());

	if (hMutex == nullptr)
	{
		return Handle::Null;
	}

	return CreateHandle<Handle>(hMutex);
}

bool Mutex::RawRemove(std::string_view name)
{
	// there is no way to do this in windows
	return true;
}

bool Mutex::RawTryLock(Handle handle)
{
	return WaitForSingleObject(UnwrapHandle<HANDLE>(handle), 0) == WAIT_OBJECT_0;
}

bool Mutex::RawUnlock(Handle handle)
{
	return ReleaseMutex(UnwrapHandle<HANDLE>(handle));
}

bool Mutex::RawClose(Handle handle)
{
	return CloseHandle(UnwrapHandle<HANDLE>(handle));
}


bool Mutex::RawIsLocked(Handle handle)
{
	if (WaitForSingleObject(UnwrapHandle<HANDLE>(handle), 0) == WAIT_OBJECT_0)
	{
		RawUnlock(handle);
		return true;
	}
	return false;
}

bool Mutex::RawWait(Handle handle)
{
	return WaitForSingleObject(UnwrapHandle<HANDLE>(handle), INFINITE) == WAIT_OBJECT_0;
}


bool Mutex::RawWaitTimed(Handle handle, uint32_t timeout)
{
	return WaitForSingleObject(UnwrapHandle<HANDLE>(handle), timeout) == WAIT_OBJECT_0;
}
