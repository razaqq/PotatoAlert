// Copyright 2022 <github.com/razaqq>

#include "Core/Directory.hpp"

#include <unistd.h>

#include <filesystem>
#include <format>
#include <optional>


namespace fs = std::filesystem;

std::optional<fs::path> PotatoAlert::Core::GetModuleRootPath()
{
	char exePath[PATH_MAX + 1] = {0};
	if (readlink(std::format("/proc/{}/exe", getpid()).c_str(), exePath, sizeof(exePath)) == -1)
	{
		return {};
	}
	return fs::path(exePath).remove_filename();
}
