// Copyright 2025 <github.com/razaqq>

#include "Core/StandardPaths.hpp"

#include "Core/Directory.hpp"
#include "Core/Result.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <filesystem>
#include <string>
#include <system_error>


namespace c = PotatoAlert::Core;
namespace fs = std::filesystem;
using PotatoAlert::Core::Result;

Result<std::filesystem::path> c::AppDataPath()
{
	const char* home;
	if ((home = getenv("HOME")) == nullptr)
	{
		passwd* pw = getpwuid(getuid());
		if (pw == nullptr)
		{
			return PA_ERROR(std::error_code(errno, std::system_category()));
		}
		home = pw->pw_dir;
	}

	return home;
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
	return P_tmpdir;
}
