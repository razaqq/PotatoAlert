// Copyright 2022 <github.com/razaqq>

#include "Mutex.hpp"

#include <semaphore.h>


using PotatoAlert::Core::Mutex;

std::string Mutex::RawLastError(Handle handle)
{
	return "";
}

Mutex::Handle Mutex::RawCreate(std::string_view name, bool initiallyOwned)
{
	return Handle::Null;
}

Mutex::Handle Mutex::RawOpen(std::string_view name)
{
	return Handle::Null;
}

bool Mutex::RawTryLock(Handle handle)
{
	return false;
}

bool Mutex::RawUnlock(Handle handle)
{
	return false;
}

bool Mutex::RawClose(Handle handle)
{
	return false;
}


bool Mutex::RawIsLocked(Handle handle)
{
	return false;
}

bool Mutex::RawWait(Handle handle, uint32_t timeout)
{
	return false;
}
