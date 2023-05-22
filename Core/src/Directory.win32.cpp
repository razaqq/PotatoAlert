// Copyright 2022 <github.com/razaqq>

#include "Core/Directory.hpp"
#include "Core/Result.hpp"

#include "win32.h"

#include <filesystem>


namespace fs = std::filesystem;
using PotatoAlert::Core::Result;

Result<fs::path> PotatoAlert::Core::GetModuleRootPath()
{
	char path[MAX_PATH];
	if (!GetModuleFileNameA(nullptr, path, MAX_PATH))
	{
		return PA_ERROR(std::error_code(GetLastError(), std::system_category()));
	}
	return fs::path(path).remove_filename();
}
