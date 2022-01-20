// Copyright 2021 <github.com/razaqq>

#include "Process.hpp"

#define WIN32_SHOWWINDOW
#define WIN32_USER
#include "win32.h"

#include <shellapi.h>


namespace pp = PotatoAlert::Process;

bool pp::CreateNewProcess(std::string_view path, std::string_view args, bool elevated)
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
