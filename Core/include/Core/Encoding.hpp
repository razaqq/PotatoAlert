// Copyright 2023 <github.com/razaqq>
#pragma once

#include "Core/Result.hpp"

#include <filesystem>
#include <string>
#include <span>


namespace PotatoAlert::Core {

Result<size_t> Utf8ToWide(std::string_view string, std::span<wchar_t> buffer);
Result<size_t> Utf8ToWide(std::string_view string);

Result<size_t> WideToUtf8(std::wstring_view string, std::span<char> buffer);
Result<size_t> WideToUtf8(std::wstring_view string);

Result<std::string> NativeToUtf8(std::string_view string);

Result<std::string> PathToUtf8(const std::filesystem::path& path);
Result<std::filesystem::path> Utf8ToPath(std::string_view string);

}  // namespace PotatoAlert::Core
