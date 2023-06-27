// Copyright 2022 <github.com/razaqq>

#include "Core/Directory.hpp"
#include "Core/Result.hpp"

#include <linux/limits.h>
#include <unistd.h>

#include <filesystem>
#include <format>
#include <optional>


namespace fs = std::filesystem;

PotatoAlert::Core::Result<fs::path> PotatoAlert::Core::GetModuleRootPath()
{
	char exePath[PATH_MAX + 1] = {0};
	if (readlink(std::format("/proc/{}/exe", getpid()).c_str(), exePath, sizeof(exePath)) == -1)
	{
		return PA_ERROR(std::error_code(errno, std::system_category()));
	}
	return fs::path(exePath).remove_filename();
}
