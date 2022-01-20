#include "Directory.hpp"

#include "win32.h"

#include <filesystem>


namespace fs = std::filesystem;

std::optional<fs::path> PotatoAlert::GetModuleRootPath()
{
	char path[MAX_PATH];
	if (!GetModuleFileNameA(nullptr, path, MAX_PATH))
	{
		return {};
	}
	return fs::path(path).remove_filename();
}
