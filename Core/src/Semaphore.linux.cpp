// Copyright 2022 <github.com/razaqq>

#include "Core/Semaphore.hpp"

#include <fcntl.h>
#include <semaphore.h>

#include <string>


using PotatoAlert::Core::Semaphore;

namespace {

template<typename T>
static constexpr T CreateHandle(sem_t* handle)
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
	return "";
}

Semaphore::Handle Semaphore::RawCreate(std::string_view name, int initialValue)
{
	sem_t* s = sem_open(name.data(), O_CREAT | O_EXCL, 0755, initialValue);
	if (s == SEM_FAILED)
	{
		return Handle::Null;
	}

	return CreateHandle<Handle>(s);
}

Semaphore::Handle Semaphore::RawOpen(std::string_view name)
{
	sem_t* s = sem_open(name.data(), 0);
	if (s == SEM_FAILED)
	{
		return Handle::Null;
	}

	return CreateHandle<Handle>(s);
}

bool Semaphore::RawRemove(std::string_view name)
{
	return sem_unlink(name.data()) == 0;
}

bool Semaphore::RawTryLock(Handle handle)
{
	return sem_trywait(UnwrapHandle<sem_t*>(handle)) == 0;
}

bool Semaphore::RawUnlock(Handle handle)
{
	return sem_post(UnwrapHandle<sem_t*>(handle)) == 0;
}

bool Semaphore::RawClose(Handle handle)
{
	return sem_close(UnwrapHandle<sem_t*>(handle)) == 0;
}

bool Semaphore::RawIsLocked(Handle handle)
{
	int value;
	if (sem_getvalue(UnwrapHandle<sem_t*>(handle), &value) == -1)
	{
		return false;
	}
	return value <= 0;
}

bool Semaphore::RawWait(Handle handle)
{
	return sem_wait(UnwrapHandle<sem_t*>(handle)) == 0;
}

bool Semaphore::RawWaitTimed(Handle handle, uint32_t timeout)
{
	struct timespec ts{};
	ts.tv_nsec = static_cast<long>(timeout * 1e6);
	return sem_timedwait(UnwrapHandle<sem_t*>(handle), &ts) == 0;
}
