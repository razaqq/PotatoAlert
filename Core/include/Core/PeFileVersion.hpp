// Copyright 2024 <github.com/razaqq>
#pragma once

#include "Core/PeReader.hpp"
#include "Core/Result.hpp"

#include <filesystem>
#include <variant>


namespace PotatoAlert::Core {

enum class FileVersionReadErrorMisc
{
	FileOpenError,
	FileMapError,
	MissingResourceTable,
	MissingResourceVersionEntry,
};

using FileVersionReadError = std::variant<PeError, FileVersionReadErrorMisc>;

std::string_view GetErrorMessage(FileVersionReadError error);
Result<Version, FileVersionReadError> ReadFileVersion(const std::filesystem::path& p);

}  // namespace PotatoAlert::Core
