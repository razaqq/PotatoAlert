// Copyright 2022 <github.com/razaqq>
#pragma once

#include "Core/Process.hpp"
#include "Core/Result.hpp"
#include "Core/StandardPaths.hpp"
#include "Core/String.hpp"

#include <filesystem>
#include <format>
#include <iostream>
#include <string>

#include <fmt/xchar.h>
#include <fmt/std.h>


namespace fs = std::filesystem;
using PotatoAlert::Core::Result;

namespace PotatoAlert::Client {

struct AppDirectories
{
public:
	explicit AppDirectories(std::string_view appName)
	{
		AppName = appName;
		AppDir = Core::AppDataPath(appName).make_preferred();
		MatchesDir = (AppDir / "Matches").make_preferred();
		ScreenshotsDir = (AppDir / "Screenshots").make_preferred();
		ReplayVersionsDir = (AppDir / "ReplayVersions").make_preferred();
		ConfigFile = (AppDir / "config.json").make_preferred();
		LogFile = (AppDir / std::format("{}.log", AppName)).make_preferred();
		DatabaseFile = (MatchesDir / "match_history.db").make_preferred();

		CreateAppDir(MatchesDir);
		CreateAppDir(ScreenshotsDir);
		CreateAppDir(ReplayVersionsDir);
	}

	std::string_view AppName;
	fs::path AppDir;

	fs::path MatchesDir;
	fs::path ScreenshotsDir;
	fs::path ReplayVersionsDir;
	fs::path ConfigFile;
	fs::path LogFile;
	fs::path DatabaseFile;

private:
	static void CreateAppDir(const fs::path& dir)
	{
		std::error_code ec;
		const bool exists = fs::exists(dir, ec);
		if (ec)
		{
			fmt::println(stderr, STR("Failed to check if app dir '{}' exists, reason: '{}'"), dir, ec);
			Core::ExitCurrentProcessWithError(ec.value());
		}

		if (!exists)
		{
			fs::create_directories(dir, ec);
			if (ec)
			{
				fmt::println(stderr, STR("Failed to create app dir '{}', reason: '{}'"), dir, ec);
				Core::ExitCurrentProcessWithError(ec.value());
			}
		}
	}
};

}  // namespace PotatoAlert::Client
