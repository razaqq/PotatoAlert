// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Core/Result.hpp"

#include <filesystem>


namespace PotatoAlert::Core {

Result<std::filesystem::path> GetModuleRootPath();

}  // namespace PotatoAlert::Core
