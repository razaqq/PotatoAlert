// Copyright 2022 <github.com/razaqq>

#include "Core/Directory.hpp"

#include "Core/Format.hpp"
#include "Core/Result.hpp"

#include <filesystem>


namespace fs = std::filesystem;
using PotatoAlert::Core::Result;

Result<bool> PotatoAlert::Core::PathExists(const fs::path& path)
{
	std::error_code ec;
	bool exists = fs::exists(path, ec);
	if (ec)
	{
		return PA_ERROR(ec);
	}

	return exists;
}

Result<bool> PotatoAlert::Core::IsSubdirectory(const std::filesystem::path& path, const std::filesystem::path& root)
{
	fs::path current = path;
	while (current.has_relative_path())
	{
		std::error_code ec;
		const bool eq = fs::equivalent(current, root, ec);
		if (ec)
		{
			return PA_ERROR(ec);
		}
		if (eq)
		{
			return true;
		}
		current = current.parent_path();
	}
	return fs::equivalent(current, root);
}

