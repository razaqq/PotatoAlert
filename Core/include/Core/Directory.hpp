// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Core/Result.hpp"

#include <filesystem>


namespace PotatoAlert::Core {

Result<bool> PathExists(const std::filesystem::path& path);
Result<void> CreatePath(const std::filesystem::path& path);
Result<bool> IsSubdirectory(const std::filesystem::path& path, const std::filesystem::path& root);
Result<std::filesystem::path> GetModuleRootPath();

static_assert(
	std::is_same_v<std::filesystem::path::value_type, char> ||
	std::is_same_v<std::filesystem::path::value_type, wchar_t>,
	"Unsupported char type std::filesystem::path::value_type"
);

}  // namespace PotatoAlert::Core
