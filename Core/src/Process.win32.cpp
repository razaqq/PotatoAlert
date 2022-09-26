// Copyright 2021 <github.com/razaqq>

#include "Core/Process.hpp"

#define WIN32_SHOWWINDOW
#define WIN32_USER
#define WIN32_MB
#include "win32.h"

#include <shellapi.h>
#include <string>


namespace c = PotatoAlert::Core;

void c::ExitCurrentProcess(uint32_t code)
{
	ExitProcess(code);
}

void PotatoAlert::Core::ExitCurrentProcessWithError(uint32_t code)
{
	MessageBox(
		nullptr,
		L"A critical error has occurred, please view the logs and report the error to the developer.",
		L"Critical Error",
		MB_OK | MB_ICONERROR | MB_SETFOREGROUND
	);
	ExitProcess(code);
}

bool c::CreateNewProcess(std::string_view path, std::string_view args, bool elevated)
{
	const char* lpVerb = elevated ? "runas" : "open";
	const std::string pathStr = std::string(path);
	const std::string argsStr = std::string(args);
	SHELLEXECUTEINFOA sei = {
		sizeof(sei),
		SEE_MASK_NO_CONSOLE,
		nullptr,
		lpVerb,
		pathStr.c_str(),
		argsStr.c_str(),
		nullptr,
		SW_SHOWNORMAL
	};
	return ShellExecuteExA(&sei);
}
