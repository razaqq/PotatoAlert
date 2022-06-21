// Copyright 2022 <github.com/razaqq>
#pragma once

#include <filesystem>
#include <optional>


namespace PotatoAlert::Core {

std::optional<std::filesystem::path> GetModuleRootPath();

}  // namespace PotatoAlert::Core
