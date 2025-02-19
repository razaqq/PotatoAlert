// Copyright 2021 <github.com/razaqq>
#pragma once

#include <cstdint>
#include <filesystem>
#include <string>


namespace PotatoAlert::Core {

[[noreturn]] void ExitCurrentProcess(uint32_t code);
[[noreturn]] void ExitCurrentProcessWithError(uint32_t code);
bool CreateNewProcess(const std::filesystem::path& path, std::string_view args, bool elevated);

}  // namespace PotatoAlert::Core
