// Copyright 2025 <github.com/razaqq>

#include "Core/Directory.hpp"
#include "Core/StandardPaths.hpp"
#include "Core/Result.hpp"

#include <Windows.h>
#include <shlobj_core.h>

#include <filesystem>
#include <string>
#include <system_error>


namespace c = PotatoAlert::Core;
namespace fs = std::filesystem;
using PotatoAlert::Core::Result;

Result<std::filesystem::path> c::AppDataPath()
{
	wchar_t path[MAX_PATH];

	if (SHGetSpecialFolderPathW(nullptr, path, CSIDL_LOCAL_APPDATA, TRUE) != TRUE)
	{
		return PA_ERROR(std::error_code(GetLastError(), std::system_category()));
	}

	return path;
}

Result<std::filesystem::path> c::AppDataPath(std::string_view appName)
{
	PA_TRY(appData, AppDataPath());
	const fs::path full = appData / appName.data();
	PA_TRY(exists, PathExists(full));
	if (!exists)
	{
		PA_TRYV(CreatePath(full));
	}
	return full;
}

Result<std::filesystem::path> c::TempPath()
{
	wchar_t path[MAX_PATH];

	if (GetTempPathW(MAX_PATH, path) == NULL)
	{
		return PA_ERROR(std::error_code(GetLastError(), std::system_category()));
	}

	return path;
}
