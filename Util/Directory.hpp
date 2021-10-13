#pragma once

#include <filesystem>
#include <optional>


namespace PotatoAlert {

std::optional<std::filesystem::path> GetModuleRootPath();

}  // namespace PotatoAlert::Dir