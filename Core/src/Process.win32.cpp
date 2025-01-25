// Copyright 2021 <github.com/razaqq>

#include "Core/Encoding.hpp"
#include "Core/Process.hpp"
#include "Core/Result.hpp"

#define WIN32_SHOWWINDOW
#define WIN32_USER
#define WIN32_MB
#include "win32.h"

#include <shellapi.h>

#include <cstdint>
#include <filesystem>
#include <string>


namespace c = PotatoAlert::Core;

void c::ExitCurrentProcess(uint32_t code)
{
	ExitProcess(code);
}

void PotatoAlert::Core::ExitCurrentProcessWithError(uint32_t code)
{
	MessageBoxW(
		nullptr,
		L"A critical error has occurred, please view the logs and report the error to the developer.",
		L"Critical Error",
		MB_OK | MB_ICONERROR | MB_SETFOREGROUND
	);
	ExitProcess(code);
}

bool c::CreateNewProcess(const std::filesystem::path& path, std::string_view args, bool elevated)
{
	const wchar_t* lpVerb = elevated ? L"runas" : L"open";
	const std::wstring& pathStr = path.native();

	Result<size_t> size = Utf8ToWide(args);
	if (!size)
	{
		return false;
	}
	std::wstring argsStr(size.value(), L'\0');
	if (!Utf8ToWide(args, argsStr))
	{
		return false;
	}

	SHELLEXECUTEINFOW sei = {};  // TODO: use W version here
	sei.cbSize = sizeof(SHELLEXECUTEINFOW);
	sei.fMask = SEE_MASK_NO_CONSOLE;
	sei.lpVerb = lpVerb;
	sei.lpFile = pathStr.c_str();
	sei.lpParameters = argsStr.c_str();
	sei.lpDirectory = nullptr;
	sei.nShow = SW_SHOWNORMAL;

	return ShellExecuteExW(&sei);
}
