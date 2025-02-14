// Copyright 2022 <github.com/razaqq>
#pragma once

#include <Core/Result.hpp>

#include <filesystem>
#include <string>


namespace PotatoAlert::Core {

Result<std::filesystem::path> AppDataPath();
Result<std::filesystem::path> AppDataPath(std::string_view appName);
Result<std::filesystem::path> TempPath();

}  // namespace PotatoAlert::Core
