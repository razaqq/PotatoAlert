// Copyright 2022 <github.com/razaqq>

#include "Core/ApplicationGuard.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <string>


using PotatoAlert::Core::ApplicationGuard;

namespace {

template<typename T>
static constexpr T CreateHandle(int handle)
{
	return static_cast<T>(static_cast<uintptr_t>(handle) + 1);
}

template<typename T>
static constexpr T UnwrapHandle(ApplicationGuard::Handle handle)
{
	return static_cast<T>(static_cast<uintptr_t>(handle) - 1);
}

}  // namespace

ApplicationGuard::ApplicationGuard(std::string name) : m_name(std::move(name))
{
	m_handle = CreateHandle<Handle>(open(("/tmp/" + m_name).c_str(), O_RDWR | O_CREAT | O_CLOEXEC, S_IRUSR | S_IWUSR));
}

ApplicationGuard::~ApplicationGuard()
{
	lockf(UnwrapHandle<int>(m_handle), F_UNLCK, 0);
	close(UnwrapHandle<int>(m_handle));
}

bool ApplicationGuard::Reset()
{
	const std::string file = "/tmp/" + m_name;
	close(UnwrapHandle<int>(m_handle));
	remove(file.c_str());
	m_handle = CreateHandle<Handle>(open(file.c_str(), O_RDWR | O_CREAT | O_CLOEXEC, S_IRUSR | S_IWUSR));

	return lockf(UnwrapHandle<int>(m_handle), F_TLOCK, 0) == 0;
}

bool ApplicationGuard::ExistsOtherInstance() const
{
	if (!lockf(UnwrapHandle<int>(m_handle), F_TLOCK, 0))
		return false;

	switch (errno)
	{
		case EACCES:
		case EAGAIN:
			return true;
		default:
			return false;
	}
}
