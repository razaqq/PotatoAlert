// Copyright 2022 <github.com/razaqq>

#include "Core/ApplicationGuard.hpp"

#include "win32.h"

#include <string>



using PotatoAlert::Core::ApplicationGuard;

namespace {

template<typename T>
static constexpr T CreateHandle(HANDLE handle)
{
	return static_cast<T>(reinterpret_cast<uintptr_t>(handle) + 1);
}

template<typename T>
static constexpr T UnwrapHandle(ApplicationGuard::Handle handle)
{
	return reinterpret_cast<T>(static_cast<uintptr_t>(handle) - 1);
}

}  // namespace

ApplicationGuard::ApplicationGuard(std::string name) : m_name(std::move(name))
{
	m_handle = CreateHandle<Handle>(CreateMutexA(nullptr, true, m_name.c_str()));
}

ApplicationGuard::~ApplicationGuard()
{
	ReleaseMutex(UnwrapHandle<HANDLE>(m_handle));
	CloseHandle(UnwrapHandle<HANDLE>(m_handle));
}

bool ApplicationGuard::Reset()
{
	// You cant delete a mutex you don't own on windows, you would have to close all handles to it, which you cant
	return false;
}

bool ApplicationGuard::ExistsOtherInstance() const
{
	return WaitForSingleObject(UnwrapHandle<HANDLE>(m_handle), 0) == WAIT_TIMEOUT;
}
